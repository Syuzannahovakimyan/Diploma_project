#include "matrix_compute.h"

Matrix_compute::Matrix_compute(const Matrix& A, const std::vector<int>& Bx,const std::vector<int>& By) {
    Eigen::MatrixXd _A(A.size(),A.size());
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A.size(); ++j) {
            _A(i, j) = A[i][j];
        }
    }
    // std::cout << "Eigen Matrix A:\n" << _A << std::endl;
    // std::cout<<std::endl;

    double detA = _A.determinant();
    // std::cout<<"detA:"<<detA<<std::endl;

    Eigen::MatrixXd _Ar(A.size(),A.size());
    _Ar = _A.transpose().colwise().reverse();

    // std::cout << "Eigen Matrix Ar:\n" << _Ar << std::endl;
    // std::cout<<std::endl;

    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A.size(); ++j) {
            _A(i, j) = _Ar(i,j) / detA;
        }
    }

    // std::cout << "Eigen Matrix A:\n" << _A << std::endl;
    // std::cout<<std::endl;

    Eigen::VectorXd _Bx(Bx.size());
    for(size_t i = 0; i < Bx.size(); ++i){
        _Bx(i) = Bx[i];
    }
    // std::cout << "Eigen Matrix Bx:\n" << _Bx << std::endl;
    // std::cout<<std::endl;

    Eigen::VectorXd _By(Bx.size());
    for(size_t i = 0; i < By.size(); ++i){
        _By(i) = By[i];
    }
    // std::cout << "Eigen Matrix By:\n" << _By << std::endl;


    Eigen::VectorXd _X(_Bx.size());
    Eigen::VectorXd _Y(_Bx.size());

    _X = _A * _Bx;
    _Y = _A * _By;
    // std::cout<<std::endl;
    // std::cout << "Eigen Matrix X1:\n" << _X << std::endl;
    // std::cout<<std::endl;
    // std::cout << "Eigen Matrix Y1:\n" << _Y << std::endl;
    // std::cout<<std::endl;

    // double s = 0, h = 1;
    // for(auto i = 0; i < _X.size(); ++i){
    //     s += _X(i);
    // }
    // h = s/_X.size();
    // for(auto i = 0; i < _X.size(); ++i){
    //     _X1(i) = _X(i) - h + 50;
    // }

    // std::cout<<std::endl;
    // std::cout << "Eigen Matrix X:\n" << _X1 << std::endl;
    // std::cout<<std::endl;

    for(auto i = 0; i < _X.size(); ++i){
        x[i] = _X(i);
        y[i] = _Y(i);
    }
}

std::vector<int> Matrix_compute::print_x(){
    for(size_t i = 0; i < x.size(); ++i){
        std::cout<<x[i]<<" ";
    }
    std::cout<<std::endl;
    return x;
}

std::vector<int> Matrix_compute::print_y(){
    for(size_t i = 0; i < x.size(); ++i){
        std::cout<<y[i]<<" ";
    }
    std::cout<<std::endl;
    return y;
}

