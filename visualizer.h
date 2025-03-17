#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <QJsonArray>

class Visualizer : public QMainWindow {
    Q_OBJECT
public:
    Visualizer(QWidget *parent = nullptr);

private slots:
    void loadJson();
    void toggleCells();
    void togglePins();
    void toggleNets();

private:
    void drawGrid(int width, int height);
    void drawCells(const QJsonArray &cellsArray);
    void drawPins(const QJsonArray &cellsArray);
    void drawNets(const QJsonArray &netsArray);

    QGraphicsScene *scene;
    QGraphicsView *view;
};

#endif // VISUALIZER_H
