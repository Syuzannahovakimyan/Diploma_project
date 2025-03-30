#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <fstream>
#include <QFile>       // Ապահովում է QFile-ի ճիշտ ֆունկցիոնալությունը
#include <QIODevice>   // (եթե դեռ կա սխալը, կարող ես ավելացնել սա)
#include <QJsonDocument>
#include <cmath>
#include "controller.h"
// #include <vector>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    Controller ob;
    return a.exec();
}


