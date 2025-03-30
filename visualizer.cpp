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
#include <iostream>

Visualizer::Visualizer(QWidget *parent) : QMainWindow(parent) {
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setSceneRect(0, 0, 1000, 1000); // Enlarged scene
    view->scale(5, 5); // Scale up for better visibility

    QPushButton *loadButton = new QPushButton("Load JSON", this);
    QPushButton *toggleCells = new QPushButton("Toggle Cells", this);
    QPushButton *togglePins = new QPushButton("Toggle Pins", this);
    QPushButton *toggleNets = new QPushButton("Toggle Nets", this);
    QPushButton *togglePads = new QPushButton("Toggle Pads", this);

    connect(loadButton, &QPushButton::clicked, this, &Visualizer::loadJson);
    connect(toggleCells, &QPushButton::clicked, this, &Visualizer::toggleCells);
    connect(togglePins, &QPushButton::clicked, this, &Visualizer::togglePins);
    connect(toggleNets, &QPushButton::clicked, this, &Visualizer::toggleNets);
    connect(togglePads, &QPushButton::clicked, this, &Visualizer::togglePads);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(loadButton);
    layout->addWidget(toggleCells);
    layout->addWidget(togglePins);
    layout->addWidget(toggleNets);
    layout->addWidget(togglePads);
    layout->addWidget(view);
    // layout->addWidget(togglePads);

    std::cout<<"static"<<std::endl;

    QWidget *container = new QWidget;
    container->setLayout(layout);
    setCentralWidget(container);
}


void Visualizer::loadJson() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open JSON File", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) {
        qDebug() << "No file selected!";
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file!";
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        qDebug() << "JSON file is empty!";
        return;
    }


    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format!";
        return;
    }
    QJsonObject jsonObj = doc.object();

    scene->clear();

    int width = jsonObj["area"].toObject()["width"].toInt();
    int height = jsonObj["area"].toObject()["height"].toInt();
    drawGrid(width, height);
    drawCells(jsonObj["cells"].toArray());
    drawPins(jsonObj["cells"].toArray());
    drawNets(jsonObj["nets"].toArray());
    drawPads(jsonObj["pads"].toArray());
}

void Visualizer::drawGrid(int width, int height) {
    QPen thinPen(Qt::lightGray);
    thinPen.setCosmetic(true); // Ensures line width stays the same regardless of zoom level
    thinPen.setWidth(0);
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
    thinPen.setWidth(2);
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
    thinPen.setWidth(2);
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
void Visualizer::drawPads(const QJsonArray &padsArray) {
    QPen thinPen(Qt::green);
    thinPen.setCosmetic(true); // Ensures line width stays the same regardless of zoom level
    thinPen.setWidth(2);

    for (const auto &padVal : padsArray) {
        QJsonObject padObj = padVal.toObject();
        int x = padObj["coord"].toObject()["x"].toInt();
        int y = padObj["coord"].toObject()["y"].toInt();
        int width = padObj["size"].toObject()["width"].toInt();
        int height = padObj["size"].toObject()["height"].toInt();

        QGraphicsRectItem *rect = scene->addRect(x, y, width, height, thinPen);
        rect->setData(0, "pad");
    }
}

void Visualizer::drawNets(const QJsonArray &netsArray) {
    QPen thinPen;
    thinPen.setCosmetic(true);
    thinPen.setWidth(1);

    for (const auto &netVal : netsArray) {
        QJsonObject netObj = netVal.toObject();
        QJsonArray connections = netObj["connections"].toArray();
        if (connections.size() >= 2) {
            int x1 = connections[0].toObject()["x"].toInt();
            int y1 = connections[0].toObject()["y"].toInt();
            int x2 = connections[1].toObject()["x"].toInt();
            int y2 = connections[1].toObject()["y"].toInt();


        // int weight = netObj["weight"].toInt(); // Get the net's weight
        QString netUID = netObj["uid"].toString(); // Get the net's UID

        // Generate a random color for the net line and label
        QColor randomColor = QColor::fromRgb(QRandomGenerator::global()->generate());

        // Set the pen color to the random color
        thinPen.setColor(randomColor);

        // Draw the line with the random color
        QGraphicsLineItem *line = scene->addLine(x1, y1, x2, y2, thinPen);
        line->setData(0, "net");

        // QString label = netUID + " - " + QString::number(weight);
        QString label = netObj["uid"].toString() + " - " + QString::number(netObj["weight"].toInt());
        QGraphicsTextItem *textItem = scene->addText(label, QFont("Arial", 1));
        textItem->setDefaultTextColor(randomColor); // Set text color to match line color
        textItem->setPos((x1 + x2) / 2 - 1, (y1 + y2) / 2 - 1);
        }
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

void Visualizer::togglePads() {
    for (auto item : scene->items()) {
        if (item->data(0) == "pad")
            item->setVisible(!item->isVisible());
    }
}

