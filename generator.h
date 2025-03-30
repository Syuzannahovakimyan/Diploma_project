#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "structurs.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


using json = nlohmann::json;


class JsonDataGenerator {
public:
    JsonDataGenerator(Coord area, int cell_count, int min_cell_width, int max_cell_width,
                      int min_cell_height, int max_cell_height, int min_pin_count, int max_pin_count,
                      int pad_count, int min_pad_size, int max_pad_size);
    json generate();

private:
    size_t cell_id = 0;
    size_t pin_id = 0;
    size_t pad_id = 0;
    Coord area;
    int cell_count;
    int min_cell_width, max_cell_width;
    int min_cell_height, max_cell_height;
    int min_pin_count, max_pin_count;
    int pad_count;
    int min_pad_size, max_pad_size;
    std::mt19937 rng;
    std::vector<Cell> cells;
    std::vector<Pad> pads;
    std::unordered_map<std::string, Pin> all_pins;

    int getRandom(int min, int max);
    Cell generateCell();
    bool isOverlapping(const Cell& new_cell);
    json generateNets();
    Pad generatePad();
    Pin generatePin(const Cell& cell, std::unordered_set<Coord>& used_pins);
    bool isPadOverlapping(const Pad& new_pad);
    void generatePadsAndNets(QJsonObject &jsonObj, QJsonArray &cellsArray, QJsonArray &netsArray);
};

#endif // GENERATOR_H
