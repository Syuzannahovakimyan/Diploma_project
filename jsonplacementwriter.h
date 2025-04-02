#ifndef JSONPLACEMENTWRITER_H
#define JSONPLACEMENTWRITER_H

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "structurs.h"

class JsonPlacementWriter {
public:
    JsonPlacementWriter(const std::string& originalJsonPath,
                        const std::string& outputJsonPath);

    void writeUpdatedJson(const std::vector<Cell>& originalCells,
                          const std::vector<int>& x,
                          const std::vector<int>& y);

private:
    nlohmann::json inputJson;
    std::string outputPath;

    bool isOverlapping(int padX, int padY, int padW, int padH,
                       int cellX, int cellY, int cellW, int cellH);
};

#endif // JSONPLACEMENTWRITER_H
