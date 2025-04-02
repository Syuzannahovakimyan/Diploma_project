#include "matrix_compute.h"

Matrix_compute::Matrix_compute(const Matrix& A, const std::vector<int>& Bx,const std::vector<int>& By) {


    Eigen::MatrixXd matA(A.size(), A.size());
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A.size(); ++j) {
            matA(i, j) = A[i][j];
        }
    }

    Eigen::VectorXd vecBx(Bx.size()), vecBy(By.size());
    for (size_t i = 0; i < Bx.size(); ++i) vecBx(i) = Bx[i];
    for (size_t i = 0; i < By.size(); ++i) vecBy(i) = By[i];

    Eigen::VectorXd vecX = matA.colPivHouseholderQr().solve(vecBx);
    Eigen::VectorXd vecY = matA.colPivHouseholderQr().solve(vecBy);

    if (matA.determinant() != 0) {
        vecX = matA.colPivHouseholderQr().solve(vecBx);
        vecY = matA.colPivHouseholderQr().solve(vecBy);
    } else {
        std::cerr << "ERROR: Matrix A is singular (det=0). Cannot solve system." << std::endl;
    }


    x.resize(vecX.size());
    y.resize(vecY.size());
    for (int i = 0; i < vecX.size(); ++i) {
        x[i] = static_cast<int>(vecX(i));
        y[i] = static_cast<int>(vecY(i));
    }
}

std::vector<int> Matrix_compute::print_x() {
    for (int val : x) std::cout << val << " ";
    std::cout << std::endl;
    return x;
}

std::vector<int> Matrix_compute::print_y() {
    for (int val : y) std::cout << val << " ";
    std::cout << std::endl;
    return y;
}
