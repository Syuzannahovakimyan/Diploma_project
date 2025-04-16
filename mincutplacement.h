// mincutplacement.h
#ifndef MINCUTPLACEMENT_H
#define MINCUTPLACEMENT_H

#include "structurs.h"
#include <vector>

class MinCutPlacement {
public:
    MinCutPlacement(std::vector<Cell>& cells,
                    const std::vector<Net>& nets,
                    const std::vector<Pad>& pads,
                    const Coord& areaSize);

    void run(std::vector<int>& x, std::vector<int>& y);

private:
    std::vector<Cell>& cells;
    std::vector<Net> nets;
    std::vector<Pad> pads;
    Coord areaSize;

    void partition(std::vector<Cell*>& left, std::vector<Cell*>& right);
    int computeCutSize(const std::vector<Cell*>& left, const std::vector<Cell*>& right);
};

#endif // MINCUTPLACEMENT_H
