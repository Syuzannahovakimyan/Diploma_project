#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QJsonArray>
#include <QStatusBar>
#include <QDockWidget>
#include <QListWidget>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QMap>
#include <QGraphicsItem>
#include <QJsonObject>
#include <QJsonDocument>
#include <QColorDialog>

class CustomGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    CustomGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void positionChanged(QPointF pos);
    void itemSelected(QGraphicsItem *item);
    void jsonFileDropped(const QString &filePath);
    void scaleChanged(double scale);

private:
    QPointF lastMousePos;
    bool isPanning = false;
};

class Visualizer : public QMainWindow {
    Q_OBJECT
public:
    Visualizer(QWidget *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    void loadJson();
    void loadJsonFromFile(const QString &filePath);
    void saveCurrentPlacement();

private slots:
    // Toggle visibility
    void toggleCells();
    void togglePins();
    void toggleNets();
    void togglePads();
    void toggleLabels();
    void toggleGrid();

    // Zoom control
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setZoomLevel(double scale);

    // Layout operations
    void runPlacement();

    // Selection and information
    void handleItemSelection(QGraphicsItem *item);
    void showItemDetails(QGraphicsItem *item);

    // Color customization
    void setCellColor();
    void setPinColor();
    void setNetColor();
    void setPadColor();
    void setGridColor();

    // Element customization
    void adjustElementSize(int size);

    // Statistics update
    void updateStatistics();

    // Handle cursor position
    void updatePositionDisplay(QPointF pos);

    // Context menu
    void showContextMenu(const QPoint &pos);

private:
    // UI setup methods
    void setupUi();
    void createActions();
    void createStatusBar();
    void createToolBars();
    void createDockWidgets();
    void createConnections();

    // Drawing methods
    void drawGrid(int width, int height);
    void drawCells(const QJsonArray &cellsArray);
    void drawPins(const QJsonArray &cellsArray);
    void drawNets(const QJsonArray &netsArray);
    void drawPads(const QJsonArray &padsArray);
    void createColorLegend();

    // Helper methods
    void resetView();
    void clearSelection();
    QJsonDocument getCurrentJson() const;

    // Core UI components
    QGraphicsScene *scene;
    CustomGraphicsView *view;
    QStatusBar *statusBar;
    QLabel *positionLabel;
    QLabel *scaleLabel;

    // Button references
    QPushButton *toggleCellsBtn;
    QPushButton *togglePinsBtn;
    QPushButton *toggleNetsBtn;
    QPushButton *togglePadsBtn;
    QPushButton *toggleLabelsBtn;
    QPushButton *toggleGridBtn;
    QPushButton *loadJsonBtn;
    QPushButton *savePlacementBtn;
    QPushButton *zoomInBtn;
    QPushButton *zoomOutBtn;
    QPushButton *resetZoomBtn;
    QPushButton *runPlacementBtn;
    QPushButton *cellColorBtn;
    QPushButton *pinColorBtn;
    QPushButton *netColorBtn;
    QPushButton *padColorBtn;
    QPushButton *gridColorBtn;

    // Dock widgets
    QDockWidget *controlsDock;
    QDockWidget *infoDock;
    QDockWidget *statsDock;
    QDockWidget *legendDock;

    // Toolbars
    QToolBar *mainToolBar;
    QToolBar *viewToolBar;

    // Control widgets
    QWidget *controlsWidget;
    QWidget *infoWidget;
    QWidget *statsWidget;
    QWidget *legendWidget;

    // Information display
    QTextEdit *infoTextEdit;
    QListWidget *statsListWidget;

    // Element customization
    QSpinBox *elementSizeSpinBox;

    // Colors
    QColor cellColor = QColor(100, 100, 255, 100);
    QColor pinColor = Qt::yellow;
    QColor padColor = Qt::green;
    QColor gridColor = Qt::lightGray;
    QMap<int, QColor> netColors; // Weight-based colors

    // Element visibility
    bool showCells = true;
    bool showPins = true;
    bool showNets = true;
    bool showPads = true;
    bool showLabels = true;
    bool showGridLines = true;

    // Data
    QJsonDocument jsonDocument;
    QGraphicsItem *selectedItem = nullptr;

    // Statistics
    int totalCells = 0;
    int totalPins = 0;
    int totalNets = 0;
    int totalPads = 0;
    double avgNetWeight = 0.0;
    double totalWireLength = 0.0;
};

#endif // VISUALIZER_H
