// quadraticplacement.h
#ifndef QUADRATICPLACEMENT_H
#define QUADRATICPLACEMENT_H

#include "structurs.h"
#include <vector>

using Matrix = std::vector<std::vector<int>>;

class QuadraticPlacement
{
public:
    QuadraticPlacement(std::vector<Cell> cells,
                       std::vector<Net> nets,
                       std::vector<Pad> pads,
                       Coord areaSize);

    void compute_X(std::vector<int>& x, std::vector<int>& y);

private:
    std::vector<Cell> _cells;
    std::vector<Net> _nets;
    std::vector<Pad> _pads;
    Coord area;

    Matrix _C;
    Matrix _A;
    std::vector<int> _Bx;
    std::vector<int> _By;

    void compute_C();
    void compute_A();
    void compute_Bx_By();
    int sum(size_t i);
    bool cells_is_connected(const Cell& cell1, const Cell& cell2, Net& net) const;

    std::pair<int, Coord> get_pad_weight_and_coord(const Cell& cell);
};

#endif // QUADRATICPLACEMENT_H
