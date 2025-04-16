#include "visualizer.h"
#include "jsondatareader.h"
#include "mincutplacement.h"
#include "linesearchrouting.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QMenu>
#include <QToolButton>
#include <QStyle>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QFormLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QFontMetrics>
#include <QDrag>
#include <QUrl>
#include <QScrollBar>
#include <cmath>
#include "quadraticplacement.h"
#include "jsonplacementwriter.h"

// CustomGraphicsView implementation
CustomGraphicsView::CustomGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent) {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setMouseTracking(true);
    setAcceptDrops(true);
}

void CustomGraphicsView::wheelEvent(QWheelEvent *event) {
    // Zoom with mouse wheel
    double scaleFactor = 1.15;
    if(event->angleDelta().y() < 0) {
        scaleFactor = 1.0 / scaleFactor;
    }
    scale(scaleFactor, scaleFactor);

    // Emit scale changed signal
    double currentScale = transform().m11(); // Get current scale factor
    emit scaleChanged(currentScale);

    event->accept();
}

void CustomGraphicsView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        // Start panning with middle mouse button
        isPanning = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::LeftButton) {
        // Selection and interaction
        QGraphicsItem *item = itemAt(event->pos());
        if (item) {
            emit itemSelected(item);
        }
        QGraphicsView::mousePressEvent(event);
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void CustomGraphicsView::mouseMoveEvent(QMouseEvent *event) {
    if (isPanning) {
        // Handle panning
        QPointF delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();

        // Pan the view
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    } else {
        // Update position display
        QPointF scenePos = mapToScene(event->pos());
        emit positionChanged(scenePos);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CustomGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        // End panning
        isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void CustomGraphicsView::dragEnterEvent(QDragEnterEvent *event) {
    // Check if the drag contains URLs (files)
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CustomGraphicsView::dragMoveEvent(QDragMoveEvent *event) {
    // Allow the drag to move within the view
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CustomGraphicsView::dropEvent(QDropEvent *event) {
    // Handle file drop
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        // Only accept the first file for now
        if (!urlList.isEmpty()) {
            QString filePath = urlList.first().toLocalFile();
            if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
                emit jsonFileDropped(filePath);
                event->acceptProposedAction();
            }
        }
    }
}

// Visualizer implementation
Visualizer::Visualizer(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    createActions();
    createStatusBar();
    createToolBars();
    createDockWidgets();
    createConnections();

    // Set window properties
    setWindowTitle("IC Placement Visualizer");
    resize(1200, 800);

    // Create the color legend
    createColorLegend();

    // Initial reset
    resetView();
    view->scale(7, 7);
    loadJsonFromFile("../output.json");
    statusBar->showMessage("Genereted json file loaded", 3000);

}

void Visualizer::setupUi() {
    // Create central widget
    scene = new QGraphicsScene(this);
    view = new CustomGraphicsView(scene, this);
    view->setSceneRect(0, 0, 1000, 1000);
    view->scale(5, 5);  // Initial zoom
    setCentralWidget(view);

    // Create control widgets
    controlsWidget = new QWidget(this);
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsWidget);

    // Visibility group
    QGroupBox *visibilityGroup = new QGroupBox("Visibility", controlsWidget);
    QVBoxLayout *visibilityLayout = new QVBoxLayout(visibilityGroup);

    toggleCellsBtn = new QPushButton("Toggle Cells", visibilityGroup);
    togglePinsBtn = new QPushButton("Toggle Pins", visibilityGroup);
    toggleNetsBtn = new QPushButton("Toggle Nets", visibilityGroup);
    togglePadsBtn = new QPushButton("Toggle Pads", visibilityGroup);
    toggleLabelsBtn = new QPushButton("Toggle Labels", visibilityGroup);
    toggleGridBtn = new QPushButton("Toggle Grid", visibilityGroup);

    visibilityLayout->addWidget(toggleCellsBtn);
    visibilityLayout->addWidget(togglePinsBtn);
    visibilityLayout->addWidget(toggleNetsBtn);
    visibilityLayout->addWidget(togglePadsBtn);
    visibilityLayout->addWidget(toggleLabelsBtn);
    visibilityLayout->addWidget(toggleGridBtn);

    // File operations group
    QGroupBox *fileGroup = new QGroupBox("File Operations", controlsWidget);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);

    loadJsonBtn = new QPushButton("Load JSON", fileGroup);
    savePlacementBtn = new QPushButton("Save", fileGroup);

    // runMinCutBtn = new QPushButton("Run Min-Cut Placement", placementGroup);
    // placementLayout->addWidget(runMinCutBtn);


    fileLayout->addWidget(loadJsonBtn);
    fileLayout->addWidget(savePlacementBtn);
    // placementLayout->addWidget(runMinCutBtn);


    // Zoom controls group
    QGroupBox *zoomGroup = new QGroupBox("Zoom Controls", controlsWidget);
    QVBoxLayout *zoomLayout = new QVBoxLayout(zoomGroup);

    zoomInBtn = new QPushButton("Zoom In", zoomGroup);
    zoomOutBtn = new QPushButton("Zoom Out", zoomGroup);
    resetZoomBtn = new QPushButton("Reset Zoom", zoomGroup);

    zoomLayout->addWidget(zoomInBtn);
    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(resetZoomBtn);

    // Placement controls group
    QGroupBox *placementGroup = new QGroupBox("Placement", controlsWidget);
    QVBoxLayout *placementLayout = new QVBoxLayout(placementGroup);

    runPlacementBtn = new QPushButton("Run Quadratic Placement", placementGroup);
    placementLayout->addWidget(runPlacementBtn);

    runMinCutBtn = new QPushButton("Run Min-Cut Placement", placementGroup);
    placementLayout->addWidget(runMinCutBtn);

    // Routing controls group
    QGroupBox *routingGroup = new QGroupBox("Routing", controlsWidget);
    QVBoxLayout *routingLayout = new QVBoxLayout(routingGroup);

    // Placement controls group-ի մեջ
    runRipUpAndRerouteBtn = new QPushButton("Run Rip-up and Reroute", routingGroup);
    routingLayout->addWidget(runRipUpAndRerouteBtn);


    // Ավելացվող կոճակ
    runLineSearchRoutingBtn = new QPushButton("Run Line Search Routing", routingGroup);
    routingLayout->addWidget(runLineSearchRoutingBtn);


    // Appearance customization group
    QGroupBox *appearanceGroup = new QGroupBox("Appearance", controlsWidget);
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceGroup);

    cellColorBtn = new QPushButton("Cell Color", appearanceGroup);
    pinColorBtn = new QPushButton("Pin Color", appearanceGroup);
    netColorBtn = new QPushButton("Net Color", appearanceGroup);
    padColorBtn = new QPushButton("Pad Color", appearanceGroup);
    gridColorBtn = new QPushButton("Grid Color", appearanceGroup);

    QLabel *elementSizeLabel = new QLabel("Element Size:", appearanceGroup);
    elementSizeSpinBox = new QSpinBox(appearanceGroup);
    elementSizeSpinBox->setMinimum(1);
    elementSizeSpinBox->setMaximum(10);
    elementSizeSpinBox->setValue(3);

    QHBoxLayout *sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(elementSizeLabel);
    sizeLayout->addWidget(elementSizeSpinBox);

    appearanceLayout->addWidget(cellColorBtn);
    appearanceLayout->addWidget(pinColorBtn);
    appearanceLayout->addWidget(netColorBtn);
    appearanceLayout->addWidget(padColorBtn);
    appearanceLayout->addWidget(gridColorBtn);
    appearanceLayout->addLayout(sizeLayout);

    // Add all groups to the main controls layout
    controlsLayout->addWidget(visibilityGroup);
    controlsLayout->addWidget(fileGroup);
    controlsLayout->addWidget(zoomGroup);
    controlsLayout->addWidget(placementGroup);
    controlsLayout->addWidget(routingGroup);
    controlsLayout->addWidget(appearanceGroup);
    controlsLayout->addStretch();

    // Create info widget
    infoWidget = new QWidget(this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);

    QLabel *infoLabel = new QLabel("Element Information", infoWidget);
    infoTextEdit = new QTextEdit(infoWidget);
    infoTextEdit->setReadOnly(true);

    infoLayout->addWidget(infoLabel);
    infoLayout->addWidget(infoTextEdit);

    // Create stats widget
    statsWidget = new QWidget(this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsWidget);

    QLabel *statsLabel = new QLabel("Statistics", statsWidget);
    statsListWidget = new QListWidget(statsWidget);
    statsListWidget->addItem("Total Cells: 0");
    statsListWidget->addItem("Total Pins: 0");
    statsListWidget->addItem("Total Nets: 0");
    statsListWidget->addItem("Total Pads: 0");
    statsListWidget->addItem("Avg Net Weight: 0");
    statsListWidget->addItem("Total Wire Length: 0");

    statsLayout->addWidget(statsLabel);
    statsLayout->addWidget(statsListWidget);

    // Create legend widget
    legendWidget = new QWidget(this);
}

void Visualizer::createActions() {
    // Connect control buttons to slots
    // Visibility toggles
    connect(toggleCellsBtn, &QPushButton::clicked, this, &Visualizer::toggleCells);
    connect(togglePinsBtn, &QPushButton::clicked, this, &Visualizer::togglePins);
    connect(toggleNetsBtn, &QPushButton::clicked, this, &Visualizer::toggleNets);
    connect(togglePadsBtn, &QPushButton::clicked, this, &Visualizer::togglePads);
    connect(toggleLabelsBtn, &QPushButton::clicked, this, &Visualizer::toggleLabels);
    connect(toggleGridBtn, &QPushButton::clicked, this, &Visualizer::toggleGrid);

    // File operations
    connect(loadJsonBtn, &QPushButton::clicked, this, &Visualizer::loadJson);
    connect(savePlacementBtn, &QPushButton::clicked, this, &Visualizer::saveCurrentPlacement);

    // Zoom controls
    connect(zoomInBtn, &QPushButton::clicked, this, &Visualizer::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, this, &Visualizer::zoomOut);
    connect(resetZoomBtn, &QPushButton::clicked, this, &Visualizer::resetZoom);

    // Placement controls
    connect(runPlacementBtn, &QPushButton::clicked, this, &Visualizer::runPlacement);
    connect(runMinCutBtn, &QPushButton::clicked, this, &Visualizer::runMinCutPlacement);


    // Color customization
    connect(cellColorBtn, &QPushButton::clicked, this, &Visualizer::setCellColor);
    connect(pinColorBtn, &QPushButton::clicked, this, &Visualizer::setPinColor);
    connect(netColorBtn, &QPushButton::clicked, this, &Visualizer::setNetColor);
    connect(padColorBtn, &QPushButton::clicked, this, &Visualizer::setPadColor);
    connect(gridColorBtn, &QPushButton::clicked, this, &Visualizer::setGridColor);

    // Element size control
    connect(elementSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &Visualizer::adjustElementSize);
    connect(runLineSearchRoutingBtn, &QPushButton::clicked, this, &Visualizer::runLineSearchRouting);

    connect(runRipUpAndRerouteBtn, &QPushButton::clicked, this, &Visualizer::runRipUpAndReroute);

}

void Visualizer::createStatusBar() {
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    positionLabel = new QLabel("Position: (0, 0)", statusBar);
    scaleLabel = new QLabel("Scale: 5.0x", statusBar);

    statusBar->addPermanentWidget(positionLabel);
    statusBar->addPermanentWidget(scaleLabel);

    statusBar->showMessage("Ready");
}

void Visualizer::createToolBars() {
    mainToolBar = addToolBar("Main");
    viewToolBar = addToolBar("View");

    // Main toolbar actions
    QAction *loadAction = mainToolBar->addAction(QIcon::fromTheme("document-open"), "Load JSON");
    QAction *saveAction = mainToolBar->addAction(QIcon::fromTheme("document-save"), "Save Placement");

    connect(loadAction, &QAction::triggered, this, &Visualizer::loadJson);
    connect(saveAction, &QAction::triggered, this, &Visualizer::saveCurrentPlacement);

    // View toolbar actions
    QAction *zoomInAction = viewToolBar->addAction(QIcon::fromTheme("zoom-in"), "Zoom In");
    QAction *zoomOutAction = viewToolBar->addAction(QIcon::fromTheme("zoom-out"), "Zoom Out");
    QAction *resetZoomAction = viewToolBar->addAction(QIcon::fromTheme("zoom-original"), "Reset Zoom");

    connect(zoomInAction, &QAction::triggered, this, &Visualizer::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &Visualizer::zoomOut);
    connect(resetZoomAction, &QAction::triggered, this, &Visualizer::resetZoom);
}

void Visualizer::createDockWidgets() {
    // Controls dock
    controlsDock = new QDockWidget("Controls", this);
    controlsDock->setWidget(controlsWidget);
    controlsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, controlsDock);

    // Info dock
    infoDock = new QDockWidget("Information", this);
    infoDock->setWidget(infoWidget);
    infoDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, infoDock);

    // Stats dock
    statsDock = new QDockWidget("Statistics", this);
    statsDock->setWidget(statsWidget);
    statsDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, statsDock);

    // Legend dock
    legendDock = new QDockWidget("Legend", this);
    legendDock->setWidget(legendWidget);
    legendDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, legendDock);

    // Tab the right docks
    tabifyDockWidget(infoDock, statsDock);
    tabifyDockWidget(statsDock, legendDock);

    // Raise the info dock initially
    infoDock->raise();
}

void Visualizer::createConnections() {
    // Connect custom view signals
    connect(view, &CustomGraphicsView::positionChanged, this, &Visualizer::updatePositionDisplay);
    connect(view, &CustomGraphicsView::itemSelected, this, &Visualizer::handleItemSelection);
    connect(view, &CustomGraphicsView::jsonFileDropped, this, &Visualizer::loadJsonFromFile);
    connect(view, &CustomGraphicsView::scaleChanged, this, &Visualizer::setZoomLevel);

    // Connect context menu
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QWidget::customContextMenuRequested, this, &Visualizer::showContextMenu);
}

void Visualizer::createColorLegend() {
    QVBoxLayout *legendLayout = new QVBoxLayout(legendWidget);

    QLabel *legendTitle = new QLabel("Color Legend", legendWidget);
    legendLayout->addWidget(legendTitle);

    // Create color samples with labels
    QGridLayout *colorGrid = new QGridLayout();

    // Cell color
    QFrame *cellColorFrame = new QFrame();
    cellColorFrame->setFrameShape(QFrame::Box);
    cellColorFrame->setAutoFillBackground(true);
    QPalette cellPalette;
    cellPalette.setColor(QPalette::Window, cellColor);
    cellColorFrame->setPalette(cellPalette);
    QLabel *cellColorLabel = new QLabel("Cells");
    colorGrid->addWidget(cellColorFrame, 0, 0);
    colorGrid->addWidget(cellColorLabel, 0, 1);

    // Pin color
    QFrame *pinColorFrame = new QFrame();
    pinColorFrame->setFrameShape(QFrame::Box);
    pinColorFrame->setAutoFillBackground(true);
    QPalette pinPalette;
    pinPalette.setColor(QPalette::Window, pinColor);
    pinColorFrame->setPalette(pinPalette);
    QLabel *pinColorLabel = new QLabel("Pins");
    colorGrid->addWidget(pinColorFrame, 1, 0);
    colorGrid->addWidget(pinColorLabel, 1, 1);

    // Pad color
    QFrame *padColorFrame = new QFrame();
    padColorFrame->setFrameShape(QFrame::Box);
    padColorFrame->setAutoFillBackground(true);
    QPalette padPalette;
    padPalette.setColor(QPalette::Window, padColor);
    padColorFrame->setPalette(padPalette);
    QLabel *padColorLabel = new QLabel("Pads");
    colorGrid->addWidget(padColorFrame, 2, 0);
    colorGrid->addWidget(padColorLabel, 2, 1);

    // Net colors
    QLabel *netColorTitle = new QLabel("Net Colors (by weight):");
    colorGrid->addWidget(netColorTitle, 3, 0, 1, 2);

    // Add some sample net weights
    for (int i = 1; i <= 5; i++) {
        QColor netColor = QColor::fromHsv(i * 60 % 360, 200, 255);
        netColors[i] = netColor;

        QFrame *netColorFrame = new QFrame();
        netColorFrame->setFrameShape(QFrame::Box);
        netColorFrame->setAutoFillBackground(true);
        QPalette netPalette;
        netPalette.setColor(QPalette::Window, netColor);
        netColorFrame->setPalette(netPalette);
        QLabel *netColorLabel = new QLabel("Weight " + QString::number(i));
        colorGrid->addWidget(netColorFrame, 3 + i, 0);
        colorGrid->addWidget(netColorLabel, 3 + i, 1);
    }

    legendLayout->addLayout(colorGrid);
    legendLayout->addStretch();
}

bool Visualizer::eventFilter(QObject *obj, QEvent *event) {
    return false;  // Let the view handle events now
}

void Visualizer::loadJson() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open JSON File", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) {
        statusBar->showMessage("No file selected", 3000);
        return;
    }

    loadJsonFromFile(fileName);
}

void Visualizer::loadJsonFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        statusBar->showMessage("Failed to open file: " + filePath, 3000);
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        statusBar->showMessage("JSON file is empty: " + filePath, 3000);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        statusBar->showMessage("Invalid JSON format in: " + filePath, 3000);
        return;
    }

    jsonDocument = doc;
    QJsonObject jsonObj = doc.object();

    scene->clear();

    // Extract dimensions
    int width = jsonObj["area"].toObject()["width"].toInt();
    int height = jsonObj["area"].toObject()["height"].toInt();

    // Set appropriate scene rect
    view->setSceneRect(0, 0, width, height);

    // Draw elements
    if (showGridLines) {
        drawGrid(width, height);
    }
    drawCells(jsonObj["cells"].toArray());
    drawPins(jsonObj["cells"].toArray());
    drawNets(jsonObj["nets"].toArray());
    drawPads(jsonObj["pads"].toArray());

    // Update statistics
    updateStatistics();
    // resetZoom();
    // Status update
    statusBar->showMessage("Loaded: " + filePath, 3000);

    // Fit content in view
    resetView();
    resetZoom();
}

void Visualizer::saveCurrentPlacement() {
    if (jsonDocument.isNull()) {
        QMessageBox::warning(this, "Save Error", "No JSON data loaded to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Placement", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) {
        return;
    }

    // Get current placement data
    QJsonDocument currentJson = getCurrentJson();

    // Save to file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Save Error", "Failed to open file for writing: " + fileName);
        return;
    }

    file.write(currentJson.toJson(QJsonDocument::Indented));
    file.close();

    statusBar->showMessage("Placement saved to: " + fileName, 3000);
}

void Visualizer::toggleCells() {
    showCells = !showCells;
    for (auto item : scene->items()) {
        if (item->data(0) == "cell") {
            item->setVisible(showCells);
        }
    }
}

void Visualizer::togglePins() {
    showPins = !showPins;
    for (auto item : scene->items()) {
        if (item->data(0) == "pin") {
            item->setVisible(showPins);
        }
    }
}

void Visualizer::toggleNets() {
    showNets = !showNets;
    for (auto item : scene->items()) {
        if (item->data(0) == "net") {
            item->setVisible(showNets);
        }
    }
}

void Visualizer::togglePads() {
    showPads = !showPads;
    for (auto item : scene->items()) {
        if (item->data(0) == "pad") {
            item->setVisible(showPads);
        }
    }
}

void Visualizer::toggleLabels() {
    showLabels = !showLabels;
    for (auto item : scene->items()) {
        if (item->type() == QGraphicsTextItem::Type) {
            item->setVisible(showLabels);
        }
    }
}

void Visualizer::toggleGrid() {
    showGridLines = !showGridLines;

    // Need to redraw if grid changes
    if (!jsonDocument.isNull()) {
        QJsonObject jsonObj = jsonDocument.object();
        scene->clear();

        int width = jsonObj["area"].toObject()["width"].toInt();
        int height = jsonObj["area"].toObject()["height"].toInt();

        if (showGridLines) {
            drawGrid(width, height);
        }

        drawCells(jsonObj["cells"].toArray());
        drawPins(jsonObj["cells"].toArray());
        drawNets(jsonObj["nets"].toArray());
        drawPads(jsonObj["pads"].toArray());
    }
}

void Visualizer::zoomIn() {
    view->scale(1.2, 1.2);
    double scale = view->transform().m11();
    scaleLabel->setText("Scale: " + QString::number(scale, 'f', 1) + "x");
}

void Visualizer::zoomOut() {
    view->scale(1.0/1.2, 1.0/1.2);
    double scale = view->transform().m11();
    scaleLabel->setText("Scale: " + QString::number(scale, 'f', 1) + "x");
}

void Visualizer::resetZoom() {
    view->resetTransform();
    view->scale(5, 5);  // Initial zoom level
    scaleLabel->setText("Scale: 5.0x");
}

void Visualizer::setZoomLevel(double scale) {
    scaleLabel->setText("Scale: " + QString::number(scale, 'f', 1) + "x");
}

void Visualizer::runPlacement() {
    if (jsonDocument.isNull()) {
        QMessageBox::warning(this, "Error", "No JSON data loaded to run placement on");
        return;
    }

    statusBar->showMessage("Running quadratic placement...");

    try {
        // Get the JSON data
        QJsonObject jsonObj = jsonDocument.object();

        // Prepare data for QuadraticPlacement
        JsonDataReader reader("../output.json");  // You might need to get the actual path
        reader.readData();
        std::vector<Cell> cells = reader.getCells();
        std::vector<Net> nets = reader.getNets();
        std::vector<Pad> pads = reader.getPads();

        QuadraticPlacement placer(cells, nets, pads,{500,500});
        std::vector<int> x, y;
        placer.compute_X(x, y);


        // std::cout << "Placement results:\n";
        // for (size_t i = 0; i < cells.size(); ++i) {
        //     std::cout << "Cell " << cells[i].uid << " -> ("
        //               << x[i] << ", " << y[i] << ")\n";
        // }
        try {
            JsonPlacementWriter writer("/home/sh/diplom/build/output.json", "/home/sh/diplom/build/updated_output.json");
            writer.writeUpdatedJson(cells, x, y);
        } catch (const std::exception& e) {
            std::cerr << "Error during JSON writing: " << e.what() << std::endl;
        }

        // Redraw the scene with the new placement
        scene->clear();

        int width = jsonObj["area"].toObject()["width"].toInt();
        int height = jsonObj["area"].toObject()["height"].toInt();

        if (showGridLines) {
            drawGrid(width, height);
        }

        loadJsonFromFile("../updated_output.json");
        statusBar->showMessage("Quadratic placement completed", 3000);

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", "Fai{100,100}led to run placement: " + QString(e.what()));
        statusBar->showMessage("Placement failed", 3000);
    }
}

void Visualizer::runMinCutPlacement()
{
    if (jsonDocument.isNull()) {
        QMessageBox::warning(this, "Error", "No JSON data loaded to run placement on");
        return;
    }

    statusBar->showMessage("Running Min-Cut placement...");

    try {
        JsonDataReader reader("../output.json");  // Adjust path as necessary
        reader.readData();
        std::vector<Cell> cells = reader.getCells();
        std::vector<Net> nets = reader.getNets();
        std::vector<Pad> pads = reader.getPads();

        Coord area = {500, 500};  // Or retrieve from JSON
        std::vector<int> x, y;

        MinCutPlacement placer(cells, nets, pads, area);
        placer.run(x, y);  // Adjust method name if different

        JsonPlacementWriter writer("../output.json", "../updated_output.json");
        writer.writeUpdatedJson(cells, x, y);

        loadJsonFromFile("../updated_output.json");
        statusBar->showMessage("Min-Cut placement completed", 3000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to run Min-Cut: %1").arg(e.what()));
        statusBar->showMessage("Min-Cut failed", 3000);
    }
}


void Visualizer::runLineSearchRouting()
{
    if (jsonDocument.isNull()) {
        QMessageBox::warning(this, "Error", "No JSON data loaded for routing");
        return;
    }

    statusBar->showMessage("Running Line Search Routing...");

    // try {
    //     JsonDataReader reader("../output.json");
    //     reader.readData();
    //     std::vector<Cell> cells = reader.getCells();
    //     std::vector<Net> nets = reader.getNets();
    //     std::vector<Pad> pads = reader.getPads();

    //     Coord areaSize = {100, 100};  // Կարող ես վերցնել նաև JSON-ից

    //     LineSearchRouter router(cells, nets, pads, areaSize); // քո routing-ի class-ը
    //     router.run();  // ենթադրում եմ, որ քո մեթոդը run() է

    //     // Պահիր արդյունքները նոր JSON-ում
    //     router.saveRoutedJson("../routed_output.json");

    //     // Վերբեռնում ենք նոր JSON-ը
    //     loadJsonFromFile("../routed_output.json");

    //     statusBar->showMessage("Line Search Routing completed", 3000);
    // } catch (const std::exception& e) {
    //     QMessageBox::warning(this, "Routing Error", QString("Failed to run Line Search Routing: %1").arg(e.what()));
    //     statusBar->showMessage("Routing failed", 3000);
    // }
}
void Visualizer::runRipUpAndReroute()
{
    if (jsonDocument.isNull()) {
        QMessageBox::warning(this, "Error", "No JSON data loaded for routing");
        return;
    }

    statusBar->showMessage("Running Rip-up and Reroute...");

    // try {
    //     JsonDataReader reader("../output.json");
    //     reader.readData();
    //     std::vector<Cell> cells = reader.getCells();
    //     std::vector<Net> nets = reader.getNets();
    //     std::vector<Pad> pads = reader.getPads();

    //     Coord areaSize = {100, 100};  // կամ JSON-ից վերցրու

    //     RipUpAndRerouteRouter router(cells, nets, pads, areaSize);
    //     router.run();

    //     router.saveRoutedJson("../ripup_reroute_output.json");

    //     loadJsonFromFile("../ripup_reroute_output.json");

    //     statusBar->showMessage("Rip-up and Reroute completed", 3000);
    // } catch (const std::exception& e) {
    //     QMessageBox::warning(this, "Routing Error", QString("Failed Rip-up and Reroute: %1").arg(e.what()));
    //     statusBar->showMessage("Rip-up and Reroute failed", 3000);
    // }
}


void Visualizer::handleItemSelection(QGraphicsItem *item) {
    if (!item) return;

    // Clear previous selection
    if (selectedItem) {
        // Reset previous selection appearance
        if (selectedItem->type() == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(selectedItem);
            if (rect) {
                QPen pen = rect->pen();
                if (rect->data(0) == "cell") {
                    pen.setColor(Qt::blue);
                } else if (rect->data(0) == "pad") {
                    pen.setColor(padColor);
                }
                pen.setWidth(0);
                rect->setPen(pen);
            }
        } else if (selectedItem->type() == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(selectedItem);
            if (ellipse) {
                QPen pen = ellipse->pen();
                pen.setColor(Qt::black);
                pen.setWidth(0);
                ellipse->setPen(pen);
            }
        } else if (selectedItem->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(selectedItem);
            if (line) {
                QPen pen = line->pen();
                pen.setWidth(pen.width() / 2);
                line->setPen(pen);
            }
        }
    }

    // Set new selection
    selectedItem = item;

    // Highlight selected item
    if (item->type() == QGraphicsRectItem::Type) {
        QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
        if (rect) {
            QPen pen = rect->pen();
            pen.setColor(Qt::red);
            pen.setWidth(2);
            rect->setPen(pen);
        }
    } else if (item->type() == QGraphicsEllipseItem::Type) {
        QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
        if (ellipse) {
            QPen pen = ellipse->pen();
            pen.setColor(Qt::red);
            pen.setWidth(2);
            ellipse->setPen(pen);
        }
    } else if (item->type() == QGraphicsLineItem::Type) {
        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(item);
        if (line) {
            QPen pen = line->pen();
            pen.setWidth(pen.width() * 2);
            line->setPen(pen);
        }
    }

    // Show item details
    showItemDetails(item);
}

void Visualizer::showItemDetails(QGraphicsItem *item) {
    if (!item) {
        infoTextEdit->clear();
        return;
    }

    QString details;

    if (item->data(0) == "cell") {
        // Show cell details
        QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
        if (rect) {
            QRectF bounds = rect->rect();
            QString cellId;

            // Find the cell label
            for (auto child : scene->items()) {
                if (child->type() == QGraphicsTextItem::Type &&
                    child->data(0) == "cell" &&
                    child->sceneBoundingRect().intersects(rect->sceneBoundingRect())) {
                    QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(child);
                    cellId = textItem->toPlainText();
                    break;
                }
            }

            details = "Cell: " + cellId + "\n";
            details += "Position: (" + QString::number(bounds.x()) + ", " + QString::number(bounds.y()) + ")\n";
            details += "Size: " + QString::number(bounds.width()) + " x " + QString::number(bounds.height()) + "\n";

            // List pins in this cell
            details += "\nPins:\n";
            for (auto child : scene->items()) {
                if (child->data(0) == "pin" &&
                    child->type() == QGraphicsEllipseItem::Type &&
                    rect->sceneBoundingRect().contains(child->sceneBoundingRect().center())) {

                    QGraphicsEllipseItem *pinItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(child);
                    QPointF pinPos = pinItem->scenePos() + QPointF(pinItem->rect().width()/2, pinItem->rect().height()/2);

                    QString pinId;
                    for (auto label : scene->items()) {
                        if (label->type() == QGraphicsTextItem::Type &&
                            label->data(0) == "pin" &&
                            label->sceneBoundingRect().intersects(child->sceneBoundingRect())) {
                            QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                            pinId = textItem->toPlainText();
                            break;
                        }
                    }

                    details += "  " + pinId + " at (" + QString::number(pinPos.x()) + ", " + QString::number(pinPos.y()) + ")\n";
                }
            }
        }
    } else if (item->data(0) == "pin") {
        // Show pin details
        QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
        if (ellipse) {
            QPointF center = ellipse->scenePos() + QPointF(ellipse->rect().width()/2, ellipse->rect().height()/2);

            QString pinId;
            for (auto label : scene->items()) {
                if (label->type() == QGraphicsTextItem::Type &&
                    label->data(0) == "pin" &&
                    label->sceneBoundingRect().intersects(ellipse->sceneBoundingRect())) {
                    QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                    pinId = textItem->toPlainText();
                    break;
                }
            }

            details = "Pin: " + pinId + "\n";
            details += "Position: (" + QString::number(center.x()) + ", " + QString::number(center.y()) + ")\n";

            // Find which cell contains this pin
            for (auto cellItem : scene->items()) {
                if (cellItem->data(0) == "cell" && cellItem->type() == QGraphicsRectItem::Type) {
                    QGraphicsRectItem *cellRect = qgraphicsitem_cast<QGraphicsRectItem*>(cellItem);
                    if (cellRect->sceneBoundingRect().contains(center)) {
                        QString cellId;
                        for (auto label : scene->items()) {
                            if (label->type() == QGraphicsTextItem::Type &&
                                label->data(0) == "cell" &&
                                label->sceneBoundingRect().intersects(cellRect->sceneBoundingRect())) {
                                QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                                cellId = textItem->toPlainText();
                                break;
                            }
                        }
                        details += "Cell: " + cellId + "\n";
                        break;
                    }
                }
            }

            // Find nets connected to this pin
            details += "\nConnected Nets:\n";
            for (auto netItem : scene->items()) {
                if (netItem->data(0) == "net" && netItem->type() == QGraphicsLineItem::Type) {
                    QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(netItem);
                    QLineF netLine = line->line();
                    QPointF p1 = line->mapToScene(netLine.p1());
                    QPointF p2 = line->mapToScene(netLine.p2());

                    QRectF pinRect = ellipse->sceneBoundingRect();
                    if (pinRect.contains(p1) || pinRect.contains(p2)) {
                        QString netId;
                        int weight = line->data(1).toInt();

                        for (auto label : scene->items()) {
                            if (label->type() == QGraphicsTextItem::Type &&
                                label->data(0) == "net" &&
                                QLineF(p1, p2).center().x() - 5 < label->sceneBoundingRect().center().x() &&
                                QLineF(p1, p2).center().x() + 5 > label->sceneBoundingRect().center().x() &&
                                QLineF(p1, p2).center().y() - 5 < label->sceneBoundingRect().center().y() &&
                                QLineF(p1, p2).center().y() + 5 > label->sceneBoundingRect().center().y()) {
                                QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                                netId = textItem->toPlainText().split(" - ").first();
                                break;
                            }
                        }

                        details += "  " + netId + " (weight: " + QString::number(weight) + ")\n";
                    }
                }
            }
        }
    } else if (item->data(0) == "net") {
        // Show net details
        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(item);
        if (line) {
            QLineF netLine = line->line();
            QPointF p1 = line->mapToScene(netLine.p1());
            QPointF p2 = line->mapToScene(netLine.p2());

            QString netId;
            int weight = line->data(1).toInt();

            for (auto label : scene->items()) {
                if (label->type() == QGraphicsTextItem::Type &&
                    label->data(0) == "net" &&
                    QLineF(p1, p2).center().x() - 5 < label->sceneBoundingRect().center().x() &&
                    QLineF(p1, p2).center().x() + 5 > label->sceneBoundingRect().center().x() &&
                    QLineF(p1, p2).center().y() - 5 < label->sceneBoundingRect().center().y() &&
                    QLineF(p1, p2).center().y() + 5 > label->sceneBoundingRect().center().y()) {
                    QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                    netId = textItem->toPlainText().split(" - ").first();
                    break;
                }
            }

            details = "Net: " + netId + "\n";
            details += "Weight: " + QString::number(weight) + "\n";

            // Calculate length without ambiguous sqrt
            int dx = p2.x() - p1.x();
            int dy = p2.y() - p1.y();
            double length = std::hypot(dx, dy);

            details += "Length: " + QString::number(length) + "\n\n";

            // Find connected pins
            details += "Connected Pins:\n";
            for (auto pinItem : scene->items()) {
                if (pinItem->data(0) == "pin" && pinItem->type() == QGraphicsEllipseItem::Type) {
                    QGraphicsEllipseItem *pin = qgraphicsitem_cast<QGraphicsEllipseItem*>(pinItem);
                    QRectF pinRect = pin->sceneBoundingRect();

                    if (pinRect.contains(p1) || pinRect.contains(p2)) {
                        QString pinId;
                        for (auto label : scene->items()) {
                            if (label->type() == QGraphicsTextItem::Type &&
                                label->data(0) == "pin" &&
                                label->sceneBoundingRect().intersects(pin->sceneBoundingRect())) {
                                QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                                pinId = textItem->toPlainText();
                                break;
                            }
                        }

                        QPointF center = pin->scenePos() + QPointF(pin->rect().width()/2, pin->rect().height()/2);
                        details += "  " + pinId + " at (" + QString::number(center.x()) + ", " + QString::number(center.y()) + ")\n";
                    }
                }
            }
        }
    } else if (item->data(0) == "pad") {
        // Show pad details
        QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
        if (rect) {
            QRectF bounds = rect->rect();
            details = "Pad\n";
            details += "Position: (" + QString::number(bounds.x()) + ", " + QString::number(bounds.y()) + ")\n";
            details += "Size: " + QString::number(bounds.width()) + " x " + QString::number(bounds.height()) + "\n";

            // Find connected nets
            details += "\nConnected Nets:\n";
            // QPointF padCenter = rect->scenePos() + QPointF(bounds.width()/2, bounds.height()/2);

            for (auto netItem : scene->items()) {
                if (netItem->data(0) == "net" && netItem->type() == QGraphicsLineItem::Type) {
                    QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(netItem);
                    QLineF netLine = line->line();
                    QPointF p1 = line->mapToScene(netLine.p1());
                    QPointF p2 = line->mapToScene(netLine.p2());

                    QRectF padRect = rect->sceneBoundingRect();
                    if (padRect.contains(p1) || padRect.contains(p2)) {
                        QString netId;
                        int weight = line->data(1).toInt();

                        for (auto label : scene->items()) {
                            if (label->type() == QGraphicsTextItem::Type &&
                                label->data(0) == "net" &&
                                QLineF(p1, p2).center().x() - 5 < label->sceneBoundingRect().center().x() &&
                                QLineF(p1, p2).center().x() + 5 > label->sceneBoundingRect().center().x() &&
                                QLineF(p1, p2).center().y() - 5 < label->sceneBoundingRect().center().y() &&
                                QLineF(p1, p2).center().y() + 5 > label->sceneBoundingRect().center().y()) {
                                QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                                netId = textItem->toPlainText().split(" - ").first();
                                break;
                            }
                        }

                        details += "  " + netId + " (weight: " + QString::number(weight) + ")\n";
                    }
                }
            }
        }
    }

    infoTextEdit->setText(details);
}

void Visualizer::setCellColor() {
    QColor newColor = QColorDialog::getColor(cellColor, this, "Select Cell Color");
    if (newColor.isValid()) {
        cellColor = newColor;

        // Update color in legend
        QFrame *cellColorFrame = legendWidget->findChild<QFrame*>();
        if (cellColorFrame) {
            QPalette cellPalette;
            cellPalette.setColor(QPalette::Window, cellColor);
            cellColorFrame->setPalette(cellPalette);
        }

        // Redraw cells with new color
        for (auto item : scene->items()) {
            if (item->data(0) == "cell" && item->type() == QGraphicsRectItem::Type) {
                QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
                if (rect) {
                    QBrush brush = rect->brush();
                    brush.setColor(QColor(cellColor.red(), cellColor.green(), cellColor.blue(), 100));
                    rect->setBrush(brush);
                }
            }
        }
    }
}

void Visualizer::setPinColor() {
    QColor newColor = QColorDialog::getColor(pinColor, this, "Select Pin Color");
    if (newColor.isValid()) {
        pinColor = newColor;

        // Update color in legend
        QList<QFrame*> frames = legendWidget->findChildren<QFrame*>();
        if (frames.size() > 1) {
            QPalette pinPalette;
            pinPalette.setColor(QPalette::Window, pinColor);
            frames[1]->setPalette(pinPalette);
        }

        // Redraw pins with new color
        for (auto item : scene->items()) {
            if (item->data(0) == "pin" && item->type() == QGraphicsEllipseItem::Type) {
                QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                if (ellipse) {
                    QBrush brush = ellipse->brush();
                    brush.setColor(pinColor);
                    ellipse->setBrush(brush);
                }
            }
        }
    }
}

void Visualizer::setNetColor() {
    // For nets, we'll just randomize the colors again
    for (auto item : scene->items()) {
        if (item->data(0) == "net" && item->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(item);
            if (line) {
                int weight = line->data(1).toInt();
                QPen pen = line->pen();

                // Use weight-based color if available, otherwise random
                if (netColors.contains(weight)) {
                    pen.setColor(netColors[weight]);
                } else {
                    QColor randomColor = QColor::fromRgb(QRandomGenerator::global()->generate());
                    pen.setColor(randomColor);
                    netColors[weight] = randomColor;
                }

                line->setPen(pen);

                // Update net label color too
                for (auto label : scene->items()) {
                    if (label->type() == QGraphicsTextItem::Type &&
                        label->data(0) == "net" &&
                        QLineF(line->line().p1(), line->line().p2()).center().x() - 5 < label->sceneBoundingRect().center().x() &&
                        QLineF(line->line().p1(), line->line().p2()).center().x() + 5 > label->sceneBoundingRect().center().x() &&
                        QLineF(line->line().p1(), line->line().p2()).center().y() - 5 < label->sceneBoundingRect().center().y() &&
                        QLineF(line->line().p1(), line->line().p2()).center().y() + 5 > label->sceneBoundingRect().center().y()) {
                        QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                        textItem->setDefaultTextColor(pen.color());
                        break;
                    }
                }
            }
        }
    }

    // Update legend
    QList<QFrame*> frames = legendWidget->findChildren<QFrame*>();
    for (int i = 3; i < qMin(frames.size(), 8); i++) {
        int weight = i - 2;
        if (netColors.contains(weight)) {
            QPalette netPalette;
            netPalette.setColor(QPalette::Window, netColors[weight]);
            frames[i]->setPalette(netPalette);
        }
    }
}

void Visualizer::setPadColor() {
    QColor newColor = QColorDialog::getColor(padColor, this, "Select Pad Color");
    if (newColor.isValid()) {
        padColor = newColor;

        // Update color in legend
        QList<QFrame*> frames = legendWidget->findChildren<QFrame*>();
        if (frames.size() > 2) {
            QPalette padPalette;
            padPalette.setColor(QPalette::Window, padColor);
            frames[2]->setPalette(padPalette);
        }

        // Redraw pads with new color
        for (auto item : scene->items()) {
            if (item->data(0) == "pad" && item->type() == QGraphicsRectItem::Type) {
                QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
                if (rect) {
                    QPen pen = rect->pen();
                    pen.setColor(padColor);
                    rect->setPen(pen);
                }
            }
        }
    }
}

void Visualizer::setGridColor() {
    QColor newColor = QColorDialog::getColor(gridColor, this, "Select Grid Color");
    if (newColor.isValid()) {
        gridColor = newColor;

        // Redraw the grid with new color
        if (!jsonDocument.isNull() && showGridLines) {
            QJsonObject jsonObj = jsonDocument.object();
            int width = jsonObj["area"].toObject()["width"].toInt();
            int height = jsonObj["area"].toObject()["height"].toInt();

            // Clear and redraw all items
            scene->clear();
            drawGrid(width, height);
            drawCells(jsonObj["cells"].toArray());
            drawPins(jsonObj["cells"].toArray());
            drawNets(jsonObj["nets"].toArray());
            drawPads(jsonObj["pads"].toArray());
        }
    }
}

void Visualizer::adjustElementSize() {
    // Redraw with new element size if JSON is loaded
    if (!jsonDocument.isNull()) {
        QJsonObject jsonObj = jsonDocument.object();

        scene->clear();

        int width = jsonObj["area"].toObject()["width"].toInt();
        int height = jsonObj["area"].toObject()["height"].toInt();

        if (showGridLines) {
            drawGrid(width, height);
        }

        drawCells(jsonObj["cells"].toArray());
        drawPins(jsonObj["cells"].toArray());
        drawNets(jsonObj["nets"].toArray());
        drawPads(jsonObj["pads"].toArray());
    }
}

void Visualizer::updateStatistics() {
    if (jsonDocument.isNull()) {
        return;
    }

    QJsonObject jsonObj = jsonDocument.object();

    // Count elements
    totalCells = jsonObj["cells"].toArray().size();
    totalPads = jsonObj["pads"].toArray().size();

    totalPins = 0;
    for (const auto &cellVal : jsonObj["cells"].toArray()) {
        totalPins += cellVal.toObject()["pins"].toArray().size();
    }

    totalNets = jsonObj["nets"].toArray().size();

    // Calculate average net weight
    double totalWeight = 0.0;
    for (const auto &netVal : jsonObj["nets"].toArray()) {
        totalWeight += netVal.toObject()["weight"].toInt();
    }
    avgNetWeight = totalNets > 0 ? totalWeight / totalNets : 0.0;

    // Calculate total wire length
    totalWireLength = 0.0;
    for (const auto &netVal : jsonObj["nets"].toArray()) {
        QJsonArray connections = netVal.toObject()["connections"].toArray();
        if (connections.size() >= 2) {
            int x1 = connections[0].toObject()["x"].toInt();
            int y1 = connections[0].toObject()["y"].toInt();
            int x2 = connections[1].toObject()["x"].toInt();
            int y2 = connections[1].toObject()["y"].toInt();

            // Use hypot to avoid the sqrt ambiguity issue
            double length = std::hypot(x2 - x1, y2 - y1);
            totalWireLength += length;
        }
    }

    // Update stats view
    statsListWidget->clear();
    statsListWidget->addItem("Total Cells: " + QString::number(totalCells));
    statsListWidget->addItem("Total Pins: " + QString::number(totalPins));
    statsListWidget->addItem("Total Nets: " + QString::number(totalNets));
    statsListWidget->addItem("Total Pads: " + QString::number(totalPads));
    statsListWidget->addItem("Avg Net Weight: " + QString::number(avgNetWeight, 'f', 2));
    statsListWidget->addItem("Total Wire Length: " + QString::number(totalWireLength, 'f', 2));
}

void Visualizer::updatePositionDisplay(QPointF pos) {
    positionLabel->setText("Position: (" + QString::number(pos.x(), 'f', 1) + ", " +
                           QString::number(pos.y(), 'f', 1) + ")");
}

void Visualizer::showContextMenu(const QPoint &pos) {
    QGraphicsItem *item = view->itemAt(pos);

    if (item) {
        QMenu contextMenu(this);

        // Add actions based on item type
        if (item->data(0) == "cell") {
            contextMenu.addAction("Center View on Cell", [=]() {
                view->centerOn(item);
            });

            contextMenu.addAction("Copy Cell Info", [=]() {
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(infoTextEdit->toPlainText());
                statusBar->showMessage("Cell info copied to clipboard", 2000);
            });

            // More cell-specific actions could be added here
        } else if (item->data(0) == "pin") {
            contextMenu.addAction("Center View on Pin", [=]() {
                view->centerOn(item);
            });

            contextMenu.addAction("Highlight Connected Nets", [=]() {
                // Reset all net highlighting
                for (auto netItem : scene->items()) {
                    if (netItem->data(0) == "net" && netItem->type() == QGraphicsLineItem::Type) {
                        QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(netItem);
                        QPen pen = line->pen();
                        pen.setWidth(pen.width() / 2);
                        line->setPen(pen);
                    }
                }

                // Highlight nets connected to this pin
                QGraphicsEllipseItem *pin = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                if (pin) {
                    QRectF pinRect = pin->sceneBoundingRect();

                    for (auto netItem : scene->items()) {
                        if (netItem->data(0) == "net" && netItem->type() == QGraphicsLineItem::Type) {
                            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(netItem);
                            QLineF netLine = line->line();
                            QPointF p1 = line->mapToScene(netLine.p1());
                            QPointF p2 = line->mapToScene(netLine.p2());

                            if (pinRect.contains(p1) || pinRect.contains(p2)) {
                                QPen pen = line->pen();
                                pen.setWidth(pen.width() * 2);
                                line->setPen(pen);
                            }
                        }
                    }
                }
            });

        } else if (item->data(0) == "net") {
            contextMenu.addAction("Highlight Connected Pins", [=]() {
                // Reset all pin highlighting
                for (auto pinItem : scene->items()) {
                    if (pinItem->data(0) == "pin" && pinItem->type() == QGraphicsEllipseItem::Type) {
                        QGraphicsEllipseItem *pin = qgraphicsitem_cast<QGraphicsEllipseItem*>(pinItem);
                        QPen pen = pin->pen();
                        pen.setColor(Qt::black);
                        pen.setWidth(0);
                        pin->setPen(pen);
                    }
                }

                // Highlight pins connected to this net
                QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(item);
                if (line) {
                    QLineF netLine = line->line();
                    QPointF p1 = line->mapToScene(netLine.p1());
                    QPointF p2 = line->mapToScene(netLine.p2());

                    for (auto pinItem : scene->items()) {
                        if (pinItem->data(0) == "pin" && pinItem->type() == QGraphicsEllipseItem::Type) {
                            QGraphicsEllipseItem *pin = qgraphicsitem_cast<QGraphicsEllipseItem*>(pinItem);
                            QRectF pinRect = pin->sceneBoundingRect();

                            if (pinRect.contains(p1) || pinRect.contains(p2)) {
                                QPen pen = pin->pen();
                                pen.setColor(Qt::red);
                                pen.setWidth(2);
                                pin->setPen(pen);
                            }
                        }
                    }
                }
            });
        }

        // Add common actions
        contextMenu.addSeparator();
        contextMenu.addAction("Reset Selection", [=]() {
            clearSelection();
        });

        contextMenu.exec(view->mapToGlobal(pos));
    } else {
        // Context menu for empty space
        QMenu contextMenu(this);
        contextMenu.addAction("Reset View", this, &Visualizer::resetView);
        contextMenu.addAction("Reset Zoom", this, &Visualizer::resetZoom);
        contextMenu.exec(view->mapToGlobal(pos));
    }
}

void Visualizer::drawGrid(int width, int height) {
    QPen gridPen(gridColor);
    gridPen.setCosmetic(true);
    gridPen.setWidth(0);

    for (int x = 0; x <= width; x += 5) {
        scene->addLine(x, 0, x, height, gridPen);
    }
    for (int y = 0; y <= height; y += 5) {
        scene->addLine(0, y, width, y, gridPen);
    }
}

void Visualizer::drawCells(const QJsonArray &cellsArray) {
    QPen pen(Qt::blue, 0.3);
    pen.setCosmetic(true);
    QBrush brush(cellColor);

    for (const auto &cellVal : cellsArray) {
        QJsonObject cellObj = cellVal.toObject();
        int x = cellObj["coord"].toObject()["x"].toInt();
        int y = cellObj["coord"].toObject()["y"].toInt();
        int width = cellObj["size"].toObject()["width"].toInt();
        int height = cellObj["size"].toObject()["height"].toInt();

        QGraphicsRectItem *rect = scene->addRect(x, y, width, height, pen, brush);
        rect->setData(0, "cell");
        rect->setVisible(showCells);

        // Add cell label
        QString cellUid = cellObj["uid"].toString();
        QGraphicsTextItem *label = scene->addText(cellUid, QFont("Arial", 1));
        label->setDefaultTextColor(Qt::black);
        label->setPos(x + width/2 - 2, y + height/2 - 1);
        label->setData(0, "cell");
        label->setVisible(showCells && showLabels);
    }
}

void Visualizer::drawPins(const QJsonArray &cellsArray) {
    QPen pen(Qt::black, 0.2);
    pen.setCosmetic(true);
    QBrush brush(pinColor);

    int pinSize = elementSizeSpinBox->value() * 0.3;

    for (const auto &cellVal : cellsArray) {
        QJsonObject cellObj = cellVal.toObject();
        QJsonArray pinsArray = cellObj["pins"].toArray();
        for (const auto &pinVal : pinsArray) {
            QJsonObject pinObj = pinVal.toObject();
            int x = pinObj["coord"].toObject()["x"].toInt();
            int y = pinObj["coord"].toObject()["y"].toInt();

            QGraphicsEllipseItem *pin = scene->addEllipse(x-pinSize/2, y-pinSize/2, pinSize, pinSize, pen, brush);
            pin->setData(0, "pin");
            pin->setVisible(showPins);

            // Add pin label
            QString pinUid = pinObj["uid"].toString();
            QGraphicsTextItem *label = scene->addText(pinUid, QFont("Arial", 1));
            label->setDefaultTextColor(Qt::black);
            label->setPos(x + 0.3, y + 0.3);
            label->setData(0, "pin");
            label->setVisible(showPins && showLabels);
        }
    }
}

void Visualizer::drawNets(const QJsonArray &netsArray) {
    for (const auto &netVal : netsArray) {
        QJsonObject netObj = netVal.toObject();
        QJsonArray connections = netObj["connections"].toArray();
        if (connections.size() >= 2) {
            int x1 = connections[0].toObject()["x"].toInt();
            int y1 = connections[0].toObject()["y"].toInt();
            int x2 = connections[1].toObject()["x"].toInt();
            int y2 = connections[1].toObject()["y"].toInt();

            int weight = netObj["weight"].toInt();
            QString netUID = netObj["uid"].toString();

            // Use weight-based color if available, otherwise generate a new one
            QColor netColor;
            if (netColors.contains(weight)) {
                netColor = netColors[weight];
            } else {
                netColor = QColor::fromHsv(weight * 60 % 360, 200, 255);
                netColors[weight] = netColor;
            }

            // Vary line thickness based on weight and user setting
            double thickness = 0.1 + (weight * 0.05) * (elementSizeSpinBox->value() / 3.0);
            QPen pen(netColor, thickness);
            pen.setCosmetic(true);

            QGraphicsLineItem *line = scene->addLine(x1, y1, x2, y2, pen);
            line->setData(0, "net");
            line->setData(1, weight);
            line->setVisible(showNets);

            // Make net label
            QString label = netUID + " - " + QString::number(weight);
            QGraphicsTextItem *textItem = scene->addText(label, QFont("Arial", 1));
            textItem->setDefaultTextColor(netColor);
            textItem->setPos((x1 + x2) / 2 - 3, (y1 + y2) / 2 - 1);
            textItem->setData(0, "net");
            textItem->setVisible(showNets && showLabels);
        }
    }
}

void Visualizer::drawPads(const QJsonArray &padsArray) {
    QPen padPen(padColor);
    padPen.setCosmetic(true);
    padPen.setWidth(elementSizeSpinBox->value() * 0.6);

    for (const auto &padVal : padsArray) {
        QJsonObject padObj = padVal.toObject();
        int x = padObj["coord"].toObject()["x"].toInt();
        int y = padObj["coord"].toObject()["y"].toInt();
        int width = padObj["size"].toObject()["width"].toInt();
        int height = padObj["size"].toObject()["height"].toInt();

        QGraphicsRectItem *rect = scene->addRect(x, y, width, height, padPen);
        rect->setData(0, "pad");
        rect->setVisible(showPads);

        // Add pad label if it has a UID
        if (padObj.contains("uid")) {
            QString padUid = padObj["uid"].toString();
            QGraphicsTextItem *label = scene->addText(padUid, QFont("Arial", 1));
            label->setDefaultTextColor(Qt::darkGreen);
            label->setPos(x + width/2 - 2, y + height/2 - 1);
            label->setData(0, "pad");
            label->setVisible(showPads && showLabels);
        }
    }
}

void Visualizer::resetView() {
    if (!jsonDocument.isNull()) {
        QJsonObject jsonObj = jsonDocument.object();
        int width = jsonObj["area"].toObject()["width"].toInt();
        int height = jsonObj["area"].toObject()["height"].toInt();

        view->resetTransform();
        view->setSceneRect(0, 0, width, height);

        // Calculate the scaling factor to fit the content
        QRectF sceneRect = view->sceneRect();
        double scaleX = view->viewport()->width() / sceneRect.width();
        double scaleY = view->viewport()->height() / sceneRect.height();
        double scale = qMin(scaleX, scaleY) * 0.95;  // 95% to add some margin

        view->scale(scale, scale);
        scaleLabel->setText("Scale: " + QString::number(scale, 'f', 1) + "x");

        view->centerOn(sceneRect.center());
    } else {
        view->resetTransform();
        view->scale(5, 5);
        scaleLabel->setText("Scale: 5.0x");
    }
}

void Visualizer::clearSelection() {
    if (selectedItem) {
        // Reset selection appearance
        if (selectedItem->type() == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = qgraphicsitem_cast<QGraphicsRectItem*>(selectedItem);
            if (rect) {
                QPen pen = rect->pen();
                if (rect->data(0) == "cell") {
                    pen.setColor(Qt::blue);
                } else if (rect->data(0) == "pad") {
                    pen.setColor(padColor);
                }
                pen.setWidth(0);
                rect->setPen(pen);
            }
        } else if (selectedItem->type() == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(selectedItem);
            if (ellipse) {
                QPen pen = ellipse->pen();
                pen.setColor(Qt::black);
                pen.setWidth(0);
                ellipse->setPen(pen);
            }
        } else if (selectedItem->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = qgraphicsitem_cast<QGraphicsLineItem*>(selectedItem);
            if (line) {
                QPen pen = line->pen();
                pen.setWidth(pen.width() / 2);
                line->setPen(pen);
            }
        }

        selectedItem = nullptr;
        infoTextEdit->clear();
    }
}

QJsonDocument Visualizer::getCurrentJson() const {
    if (jsonDocument.isNull()) {
        return QJsonDocument();
    }

    // Create a copy of the current JSON
    QJsonObject updatedJson = jsonDocument.object();

    // Update cell positions based on scene items
    QJsonArray cellsArray = updatedJson["cells"].toArray();
    for (int i = 0; i < cellsArray.size(); ++i) {
        QJsonObject cellObj = cellsArray[i].toObject();
        QString cellUid = cellObj["uid"].toString();

        // Find the cell item in the scene
        for (auto item : scene->items()) {
            if (item->data(0) == "cell" && item->type() == QGraphicsRectItem::Type) {
                // Find the cell's label to compare UIDs
                for (auto label : scene->items()) {
                    if (label->type() == QGraphicsTextItem::Type &&
                        label->data(0) == "cell" &&
                        label->sceneBoundingRect().intersects(item->sceneBoundingRect())) {
                        QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                        if (textItem->toPlainText() == cellUid) {
                            // Found the matching cell, update its position
                            QGraphicsRectItem *rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(item);
                            // QRectF rect = rectItem->rect();
                            QPointF pos = rectItem->scenePos();

                            QJsonObject coordObj = cellObj["coord"].toObject();
                            coordObj["x"] = static_cast<int>(pos.x());
                            coordObj["y"] = static_cast<int>(pos.y());
                            cellObj["coord"] = coordObj;

                            // Update the pins positions relative to the cell
                            QJsonArray pinsArray = cellObj["pins"].toArray();
                            for (int j = 0; j < pinsArray.size(); ++j) {
                                QJsonObject pinObj = pinsArray[j].toObject();
                                QString pinUid = pinObj["uid"].toString();

                                // Find the matching pin
                                for (auto pinItem : scene->items()) {
                                    if (pinItem->data(0) == "pin" && pinItem->type() == QGraphicsEllipseItem::Type) {
                                        for (auto pinLabel : scene->items()) {
                                            if (pinLabel->type() == QGraphicsTextItem::Type &&
                                                pinLabel->data(0) == "pin" &&
                                                pinLabel->sceneBoundingRect().intersects(pinItem->sceneBoundingRect())) {
                                                QGraphicsTextItem *pinTextItem = qgraphicsitem_cast<QGraphicsTextItem*>(pinLabel);
                                                if (pinTextItem->toPlainText() == pinUid) {
                                                    QGraphicsEllipseItem *pinEllipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(pinItem);
                                                    QPointF pinPos = pinEllipse->scenePos() + QPointF(pinEllipse->rect().width()/2, pinEllipse->rect().height()/2);

                                                    QJsonObject pinCoordObj = pinObj["coord"].toObject();
                                                    pinCoordObj["x"] = static_cast<int>(pinPos.x());
                                                    pinCoordObj["y"] = static_cast<int>(pinPos.y());
                                                    pinObj["coord"] = pinCoordObj;

                                                    pinsArray[j] = pinObj;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            cellObj["pins"] = pinsArray;
                            cellsArray[i] = cellObj;
                            break;
                        }
                    }
                }
            }
        }
    }

    updatedJson["cells"] = cellsArray;

    // Update nets connections positions
    QJsonArray netsArray = updatedJson["nets"].toArray();
    for (int i = 0; i < netsArray.size(); ++i) {
        QJsonObject netObj = netsArray[i].toObject();
        QString netUid = netObj["uid"].toString();

        QJsonArray connections = netObj["connections"].toArray();

        // Find the net item in the scene
        for (auto item : scene->items()) {
            if (item->data(0) == "net" && item->type() == QGraphicsLineItem::Type) {
                // Find the net's label to compare UIDs
                for (auto label : scene->items()) {
                    if (label->type() == QGraphicsTextItem::Type &&
                        label->data(0) == "net" &&
                        label->sceneBoundingRect().intersects(item->sceneBoundingRect())) {
                        QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                        if (textItem->toPlainText().startsWith(netUid)) {
                            // Found the matching net, update its connections
                            QGraphicsLineItem *lineItem = qgraphicsitem_cast<QGraphicsLineItem*>(item);
                            QLineF line = lineItem->line();
                            QPointF p1 = lineItem->mapToScene(line.p1());
                            QPointF p2 = lineItem->mapToScene(line.p2());

                            // Update connection points
                            if (connections.size() >= 2) {
                                QJsonObject conn1 = connections[0].toObject();
                                QJsonObject conn2 = connections[1].toObject();

                                conn1["x"] = static_cast<int>(p1.x());
                                conn1["y"] = static_cast<int>(p1.y());
                                conn2["x"] = static_cast<int>(p2.x());
                                conn2["y"] = static_cast<int>(p2.y());

                                connections[0] = conn1;
                                connections[1] = conn2;

                                netObj["connections"] = connections;
                                netsArray[i] = netObj;
                            }

                            break;
                        }
                    }
                }
            }
        }
    }

    updatedJson["nets"] = netsArray;

    // Update pads positions
    QJsonArray padsArray = updatedJson["pads"].toArray();
    for (int i = 0; i < padsArray.size(); ++i) {
        QJsonObject padObj = padsArray[i].toObject();

        if (padObj.contains("uid")) {
            QString padUid = padObj["uid"].toString();

            // Find the pad item in the scene
            for (auto item : scene->items()) {
                if (item->data(0) == "pad" && item->type() == QGraphicsRectItem::Type) {
                    // Look for matching pad label
                    for (auto label : scene->items()) {
                        if (label->type() == QGraphicsTextItem::Type &&
                            label->data(0) == "pad" &&
                            label->sceneBoundingRect().intersects(item->sceneBoundingRect())) {
                            QGraphicsTextItem *textItem = qgraphicsitem_cast<QGraphicsTextItem*>(label);
                            if (textItem->toPlainText() == padUid) {
                                // Found matching pad, update position
                                QGraphicsRectItem *rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(item);
                                QPointF pos = rectItem->scenePos();

                                QJsonObject coordObj = padObj["coord"].toObject();
                                coordObj["x"] = static_cast<int>(pos.x());
                                coordObj["y"] = static_cast<int>(pos.y());
                                padObj["coord"] = coordObj;

                                padsArray[i] = padObj;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    updatedJson["pads"] = padsArray;

    return QJsonDocument(updatedJson);
}
