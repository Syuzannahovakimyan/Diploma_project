#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "jsondatareader.h"
#include "quadraticplacement.h"
#include "visualizer.h"
#include "generator.h"
#include "jsonplacementwriter.h"

class Controller
{
public:
    Controller();
    Visualizer *v;
};

#endif // CONTROLLER_H
