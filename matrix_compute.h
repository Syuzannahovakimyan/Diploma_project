#ifndef MATRIX_COMPUTE_H
#define MATRIX_COMPUTE_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>

using Matrix = std::vector<std::vector<int>>;

class Matrix_compute
{
public:
    Matrix_compute(const Matrix& A, const std::vector<int>& Bx, const std::vector<int>& By);
    std::vector<int> print_x();
    std::vector<int> print_y();
private:
    std::vector<int> x;
    std::vector<int> y;
};

#endif // MATRIX_COMPUTE_H
