#ifndef QUADRATICPLACEMENT_H
#define QUADRATICPLACEMENT_H
#include "structurs.h"
#include <vector>
#include "matrix_compute.h"


using Matrix = std::vector<std::vector<int>>;

class QuadraticPlacement
{
public:
    QuadraticPlacement(std::vector<Cell>, std::vector<Net>, std::vector<Pad>);
    void compute_X(std::vector<int>& _x, std::vector<int>& _y);
    void compute_A();
    void compute_C();
    void compute_Bx_By();
    bool cells_is_connected(const Cell& cell1, const Cell& cell2, Net& net) const;


private:
    std::vector<Cell> _cells;
    std::vector<Net> _nets;
    std::vector<Pad> _pads;
    void print_cells();
    void print_Matrix(Matrix );
    void print_Vector(std::vector<int>);
    int sum(size_t i);
    int pad_waigth(const Cell& cell, Pad& pad );


    Matrix _C;
    Matrix _A;
    std::vector<int> _Bx;
    std::vector<int> _By;

};

#endif // QUADRATICPLACEMENT_H
