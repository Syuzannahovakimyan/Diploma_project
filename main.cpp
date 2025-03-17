#include "mainwindow.h"
#include "generator.h"
#include <QApplication>
#include <iostream>
#include <fstream>
#include "visualizer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    Coord area = {100, 100};
    JsonDataGenerator generator(area, 10, 1, 5, 1, 5, 1, 3);
    json jsonData = generator.generate();
    std::ofstream file("../output.json");
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
        std::cout << "JSON data generated successfully and saved to output.json!" << std::endl;
    } else {
        std::cerr << "Failed to open output.json for writing!" << std::endl;
    }
    Visualizer window;
    window.show();

    w.show();
    return a.exec();
}
