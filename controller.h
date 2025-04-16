#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "jsondatareader.h"
#include "quadraticplacement.h"
#include "visualizer.h"
#include "generator.h"
#include "jsonplacementwriter.h"
#include "mincutplacement.h"
#include "linesearchrouting.h"
class Controller
{
public:
    Controller();
    Visualizer *v;
    Coord getAdjustedPinPosition(const Pin& pin, const std::vector<Cell>& cells, const std::vector<Pad>& pads);
};

#endif // CONTROLLER_H
