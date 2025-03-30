#ifndef JSONDATAREADER_H
#define JSONDATAREADER_H

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "structurs.h"

using json = nlohmann::json;

class JsonDataReader {
public:
    JsonDataReader(const std::string& filename);
    void readData();

    std::vector<Cell> getCells() const { return cells; }
    std::vector<Net> getNets() const { return nets; }
    std::vector<Pad> getPads() const {return pads;}

private:
    std::string filename;
    std::vector<Cell> cells;
    std::vector<Net> nets;
    std::vector<Pad> pads;
};
#endif // JSONDATAREADER_H
