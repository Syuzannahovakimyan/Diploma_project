#include "visualizer.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>

Visualizer::Visualizer(QWidget *parent) : QMainWindow(parent) {
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setSceneRect(0, 0, 1000, 1000); // Enlarged scene
    view->scale(10, 10); // Scale up for better visibility

    QPushButton *loadButton = new QPushButton("Load JSON", this);
    QPushButton *toggleCells = new QPushButton("Toggle Cells", this);
    QPushButton *togglePins = new QPushButton("Toggle Pins", this);
    QPushButton *toggleNets = new QPushButton("Toggle Nets", this);

    connect(loadButton, &QPushButton::clicked, this, &Visualizer::loadJson);
    connect(toggleCells, &QPushButton::clicked, this, &Visualizer::toggleCells);
    connect(togglePins, &QPushButton::clicked, this, &Visualizer::togglePins);
    connect(toggleNets, &QPushButton::clicked, this, &Visualizer::toggleNets);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(loadButton);
    layout->addWidget(toggleCells);
    layout->addWidget(togglePins);
    layout->addWidget(toggleNets);
    layout->addWidget(view);

    QWidget *container = new QWidget;
    container->setLayout(layout);
    setCentralWidget(container);
}

void Visualizer::loadJson() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open JSON File", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject jsonObj = doc.object();

    scene->clear();

    int width = jsonObj["area"].toObject()["width"].toInt();
    int height = jsonObj["area"].toObject()["height"].toInt();
    drawGrid(width, height);
    drawCells(jsonObj["cells"].toArray());
    drawPins(jsonObj["cells"].toArray());
    drawNets(jsonObj["nets"].toArray());
}

void Visualizer::drawGrid(int width, int height) {
    QPen thinPen(Qt::lightGray);
    thinPen.setCosmetic(true); // Ensures line width stays the same regardless of zoom level
    thinPen.setWidth(1);
    for (int x = 0; x <= width; x += 5) {
        scene->addLine(x, 0, x, height, thinPen);
    }
    for (int y = 0; y <= height; y += 5) {
        scene->addLine(0, y, width, y, thinPen);
    }
}

void Visualizer::drawCells(const QJsonArray &cellsArray) {
    QPen thinPen(Qt::blue);
    thinPen.setCosmetic(true); // Ensures line width stays the same regardless of zoom level
    thinPen.setWidth(3);
    for (const auto &cellVal : cellsArray) {
        QJsonObject cellObj = cellVal.toObject();
        int x = cellObj["coord"].toObject()["x"].toInt();
        int y = cellObj["coord"].toObject()["y"].toInt();
        int width = cellObj["size"].toObject()["width"].toInt();
        int height = cellObj["size"].toObject()["height"].toInt();

        QGraphicsRectItem *rect = scene->addRect(x, y, width, height, thinPen);
        rect->setData(0, "cell");
    }
}

void Visualizer::drawPins(const QJsonArray &cellsArray) {
    QPen thinPen(Qt::yellow);
    thinPen.setCosmetic(true); // Ensures line width stays the same regardless of zoom level
    thinPen.setWidth(1);
    for (const auto &cellVal : cellsArray) {
        QJsonObject cellObj = cellVal.toObject();
        QJsonArray pinsArray = cellObj["pins"].toArray();
        for (const auto &pinVal : pinsArray) {
            QJsonObject pinObj = pinVal.toObject();
            int x = pinObj["coord"].toObject()["x"].toInt();
            int y = pinObj["coord"].toObject()["y"].toInt();
            QGraphicsEllipseItem *pin = scene->addEllipse(x , y , 0.5, 0.5, thinPen, QBrush(Qt::yellow));
            pin->setData(0, "pin");
        }
    }
}


void Visualizer::drawNets(const QJsonArray &netsArray) {
    // Set the pen to be cosmetic (no thickness change on zoom)
    QPen thinPen;
    thinPen.setCosmetic(true);
    thinPen.setWidth(1); // Line width stays the same

    // Loop over each net in the nets array
    for (const auto &netVal : netsArray) {
        QJsonObject netObj = netVal.toObject();
        QJsonArray connections = netObj["connections"].toArray();

        if (connections.size() < 2) continue;

        // Parsing the coordinates for the connection points
        int x1 = connections[0].toObject()["pin"].toString().split("(")[1].split(",")[0].toInt();
        int y1 = connections[0].toObject()["pin"].toString().split(",")[1].split(")")[0].toInt();
        int x2 = connections[1].toObject()["pin"].toString().split("(")[1].split(",")[0].toInt();
        int y2 = connections[1].toObject()["pin"].toString().split(",")[1].split(")")[0].toInt();

        int weight = netObj["weight"].toInt(); // Get the net's weight
        QString netUID = netObj["uid"].toString(); // Get the net's UID

        // Generate a random color for the net line and label
        QColor randomColor = QColor::fromRgb(QRandomGenerator::global()->generate());

        // Set the pen color to the random color
        thinPen.setColor(randomColor);

        // Draw the line with the random color
        QGraphicsLineItem *line = scene->addLine(x1, y1, x2, y2, thinPen);
        line->setData(0, "net");

        // Create and position the label with the format "netUID - weight"
        QString label = netUID + " - " + QString::number(weight);
        QGraphicsTextItem *textItem = scene->addText(label, QFont("Arial", 1));
        textItem->setDefaultTextColor(randomColor); // Set text color to match line color
        textItem->setPos((x1 + x2) / 2 - 1, (y1 + y2) / 2 - 1); // Position the label at the center of the line
    }
}

void Visualizer::toggleCells() {
    for (auto item : scene->items()) {
        if (item->data(0) == "cell")
            item->setVisible(!item->isVisible());
    }
}

void Visualizer::togglePins() {
    for (auto item : scene->items()) {
        if (item->data(0) == "pin")
            item->setVisible(!item->isVisible());
    }
}

void Visualizer::toggleNets() {
    for (auto item : scene->items()) {
        if (item->data(0) == "net")
            item->setVisible(!item->isVisible());
    }
}
