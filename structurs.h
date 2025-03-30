#ifndef STRUCTURS_H
#define STRUCTURS_H

#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>



using json = nlohmann::json;


struct Coord {
    int x, y;
    bool operator==(const Coord& other) const {
        return x == other.x && y == other.y;
    }
};




namespace std {
template<>
struct hash<Coord> {
    size_t operator()(const Coord& coord) const {
        return hash<int>()(coord.x) ^ hash<int>()(coord.y);
    }
};
}

struct Pin {
    std::string uid;
    Coord coord;
};

struct Net {
    std::string uid;
    std::pair<Pin, Pin> connections;
    int weight;
};

struct Cell {
    std::string uid;
    Coord coord;
    int width, height;
    std::vector<Pin> pins;
    bool operator==(const Cell& other) const {
        return uid == other.uid && coord == other.coord && width == other.width && height == other.height;
    }

    bool net_is_exist(const Net& net) const{
        for(auto i : pins){
            if(i.uid == net.connections.first.uid
                || i.uid == net.connections.second.uid   ){
                // std::cout<<"net is exist in "<<uid<<std::endl;
                return true;
            }
            // std::cout<<"net is not"<<std::endl;
        }
        return false;
    }
};

struct Pad {
    std::string uid;
    Coord coord;
    int width, height;
};


#endif // STRUCTURS_H
