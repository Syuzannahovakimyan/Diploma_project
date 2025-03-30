#include "quadraticplacement.h"
#include <iostream>

QuadraticPlacement::QuadraticPlacement(std::vector<Cell> cells,std::vector<Net> nets,std::vector<Pad> pads)
    :_cells(cells)
    ,_nets(nets)
    ,_pads(pads)
{
    _C.resize(_cells.size(),std::vector<int>(_cells.size(),0));
    _A.resize(_cells.size(),std::vector<int>(_cells.size(),0));
    _Bx.resize(_cells.size(),0);
    _By.resize(_cells.size(),0);
}

void QuadraticPlacement::compute_X(std::vector<int>& _x, std::vector<int>& _y){
    std::vector<int> x;
    std::vector<int> y;
    compute_C();
    compute_A();
    compute_Bx_By();
    Matrix_compute ob(_A,_Bx,_By);
    x = ob.print_x();
    y = ob.print_y();

}

void QuadraticPlacement::compute_C(){


    for(size_t i = 0; i < _C.size(); ++i){
        for(size_t j = i; j < _C.size(); ++j){
            Cell cell1 = _cells[i];
            Cell cell2 = _cells[j];
            if(Net result;cells_is_connected(cell1, cell2, result )){
                _C[i][j] = _C[j][i] = result.weight;
            }
            if(i == j) _C[i][j] = 0;
        }
    }
    // print_Matrix(this->_C);

}

void QuadraticPlacement::compute_A(){
    Pad pad;
    for(size_t i = 0; i < _C.size(); ++i){
        for (size_t j = i; j < _C.size(); ++j){
            if(i == j){
                _A[i][j] = sum(i) +pad_waigth(_cells[i],pad);
            }else{
            _A[i][j] = _A[j][i] =_C[i][j] * (-1);
            }
        }
    }
    // std::cout<<"/////////////////////////////////////"<<std::endl;
    // print_Matrix(_A);
}

void QuadraticPlacement::compute_Bx_By(){
    Pad pad;
    for(size_t i = 0; i < _Bx.size(); ++i){
            _Bx[i] = pad_waigth(_cells[i],pad ) * pad.coord.x;
            _By[i] = pad_waigth(_cells[i],pad ) * pad.coord.y;

    }
    // std::cout<<"/////////////////////////////////////"<<std::endl;
    // print_Vector(_Bx);
    // std::cout<<"/////////////////////////////////////"<<std::endl;
    // print_Vector(_By);
}

int QuadraticPlacement::sum(size_t i){
    int s = 0;
    for(size_t j = 0; j < _C[i].size(); ++j){
        s += _C[i][j];
    }
    return s;
}

int QuadraticPlacement::pad_waigth(const Cell& cell, Pad& pad){
    pad.coord.x = 0;
    pad.coord.y = 0;
    for(size_t i =0; i < _pads.size(); ++i){
        for(size_t j =0; j < _nets.size(); ++j){
            if(_nets[j].connections.first.coord.x == _pads[i].coord.x && _nets[j].connections.first.coord.y == _pads[i].coord.y){
                if(cell.net_is_exist(_nets[j])){
                    pad = _pads[i];
                    return _nets[j].weight;
                }
            }else{
                if(_nets[j].connections.second.coord.x == _pads[i].coord.x && _nets[j].connections.second.coord.y == _pads[i].coord.y){
                    if(cell.net_is_exist(_nets[j])){
                        pad = _pads[i];
                        return _nets[j].weight;
                    }
                }
            }
        }
    }
    return 0;
}

void QuadraticPlacement::print_Matrix(Matrix C ){
    for(size_t i = 0; i < C.size(); ++i){
        for(size_t j =0 ; j < C[i].size(); ++j){
            std::cout << C[i][j] << " ";
        }
        std::cout<<std::endl;
    }
}

void QuadraticPlacement::print_Vector(std::vector<int> C ){
    for(size_t i = 0; i < C.size(); ++i){
            std::cout << C[i] << " ";
        std::cout<<std::endl;
    }
}

bool QuadraticPlacement::cells_is_connected(const Cell& cell1, const Cell& cell2, Net& net) const{
    for(const auto& i : _nets){
        if(cell1.net_is_exist(i) && cell2.net_is_exist(i)){
            net = i;
            return true;
        }
    }
    return false;
}


void QuadraticPlacement::print_cells(){
    for(const auto& i : _cells){
        std::cout<< i.uid<<" ";
    }
}

