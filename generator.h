#ifndef GENERATOR_H
#define GENERATOR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>

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

struct Cell {
    std::string uid;
    Coord coord;
    int width, height;
    std::vector<Pin> pins;
};

struct NetConnection {
    std::string pin;

};

struct Net {
    std::string uid;
    std::vector<NetConnection> connections;
    int weight;
};

class JsonDataGenerator {
public:
    JsonDataGenerator(Coord area, int cell_count, int min_cell_width, int max_cell_width,
                      int min_cell_height, int max_cell_height, int min_pin_count, int max_pin_count);

    json generate();

private:
    Coord area;
    int cell_count;
    int min_cell_width, max_cell_width;
    int min_cell_height, max_cell_height;
    int min_pin_count, max_pin_count;
    std::mt19937 rng;
    std::vector<Cell> cells;
    std::unordered_map<std::string, Pin> all_pins;

    int getRandom(int min, int max);
    Cell generateCell();
    bool isOverlapping(const Cell& new_cell);
    json generateNets();
    Pin generatePin(const Cell& cell, std::unordered_set<Coord>& used_pins);
};

#endif // GENERATOR_H
