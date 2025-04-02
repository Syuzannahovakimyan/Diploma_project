#include "jsonplacementwriter.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>

using json = nlohmann::json;

JsonPlacementWriter::JsonPlacementWriter(const std::string& originalJsonPath,
                                         const std::string& outputJsonPath)
    : outputPath(outputJsonPath)
{
    std::ifstream file(originalJsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open input JSON file");
    }
    file >> inputJson;
    file.close();
}

void JsonPlacementWriter::writeUpdatedJson(const std::vector<Cell>& originalCells,
                                           const std::vector<int>& x,
                                           const std::vector<int>& y) {
    int areaW = inputJson["area"]["width"];
    int areaH = inputJson["area"]["height"];

    for (size_t i = 0; i < inputJson["cells"].size(); ++i) {
        int cellW = originalCells[i].width;
        int cellH = originalCells[i].height;

        int newX = std::max(0, std::min(x[i], areaW - cellW));
        int newY = std::max(0, std::min(y[i], areaH - cellH));

        inputJson["cells"][i]["coord"]["x"] = newX;
        inputJson["cells"][i]["coord"]["y"] = newY;

        for (auto& pin : inputJson["cells"][i]["pins"]) {
            int dx = pin["coord"]["x"].get<int>() - originalCells[i].coord.x;
            int dy = pin["coord"]["y"].get<int>() - originalCells[i].coord.y;

            pin["coord"]["x"] = newX + dx;
            pin["coord"]["y"] = newY + dy;
        }
    }

    for (auto& net : inputJson["nets"]) {
        for (auto& conn : net["connections"]) {
            std::string pinUid = conn["pin"];

            for (size_t i = 0; i < originalCells.size(); ++i) {
                for (const auto& pin : originalCells[i].pins) {
                    if (pin.uid == pinUid) {
                        int dx = pin.coord.x - originalCells[i].coord.x;
                        int dy = pin.coord.y - originalCells[i].coord.y;

                        int newX = std::max(0, std::min(x[i], areaW - originalCells[i].width));
                        int newY = std::max(0, std::min(y[i], areaH - originalCells[i].height));

                        conn["x"] = newX + dx;
                        conn["y"] = newY + dy;
                    }
                }
            }
        }
    }

    for (auto& pad : inputJson["pads"]) {
        int padX = pad["coord"]["x"];
        int padY = pad["coord"]["y"];
        int padW = pad["size"]["width"];
        int padH = pad["size"]["height"];

        for (size_t i = 0; i < originalCells.size(); ++i) {
            int cellX = std::max(0, std::min(x[i], areaW - originalCells[i].width));
            int cellY = std::max(0, std::min(y[i], areaH - originalCells[i].height));
            int cellW = originalCells[i].width;
            int cellH = originalCells[i].height;

            while (isOverlapping(padX, padY, padW, padH, cellX, cellY, cellW, cellH)) {
                padX = (padX + 5) % (areaW - padW);
                padY = (padY + 5) % (areaH - padH);
            }
        }

        pad["coord"]["x"] = padX;
        pad["coord"]["y"] = padY;
    }

    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open output JSON file");
    }
    outFile << inputJson.dump(4);
    outFile.close();
}

bool JsonPlacementWriter::isOverlapping(int padX, int padY, int padW, int padH,
                                        int cellX, int cellY, int cellW, int cellH) {
    return !(padX + padW <= cellX ||
             cellX + cellW <= padX ||
             padY + padH <= cellY ||
             cellY + cellH <= padY);
}
