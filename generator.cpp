#include "generator.h"

JsonDataGenerator::JsonDataGenerator(Coord area, int cell_count, int min_cell_width, int max_cell_width,
                                     int min_cell_height, int max_cell_height, int min_pin_count, int max_pin_count,
                                     int pad_count, int min_pad_size, int max_pad_size)
    : area(area), cell_count(cell_count), min_cell_width(min_cell_width), max_cell_width(max_cell_width),
    min_cell_height(min_cell_height), max_cell_height(max_cell_height), min_pin_count(min_pin_count), max_pin_count(max_pin_count),
    pad_count(pad_count), min_pad_size(min_pad_size), max_pad_size(max_pad_size) {
    rng.seed(std::random_device{}());
}


json JsonDataGenerator::generate() {
    json output;
    output["area"] = { {"width", area.x}, {"height", area.y} };

    cells.clear();
    pads.clear();  // Clear pads before generating new ones
    all_pins.clear();

    generateAllPads();

    // Generate cells
    while (cells.size() < static_cast<size_t>(cell_count)) {
        Cell new_cell = generateCell();
        if (!isOverlapping(new_cell)) {
            cells.push_back(new_cell);
            for (const auto& pin : new_cell.pins) {
                all_pins[pin.uid] = pin;
            }
        }
    }

    // Generate pads
    // while (pads.size() < static_cast<size_t>(pad_count)) {
    //     pads.push_back(generatePad());
    // }

    json cells_json = json::array();
    for (const auto& cell : cells) {
        json cell_json;
        cell_json["uid"] = cell.uid;
        cell_json["coord"] = { {"x", cell.coord.x}, {"y", cell.coord.y} };
        cell_json["size"] = { {"width", cell.width}, {"height", cell.height} };

        json pins_json = json::array();
        for (const auto& pin : cell.pins) {
            pins_json.push_back({ {"uid", pin.uid}, {"coord", { {"x", pin.coord.x}, {"y", pin.coord.y} }} });
        }
        cell_json["pins"] = pins_json;
        cells_json.push_back(cell_json);
    }

    output["cells"] = cells_json;

    // Add pads to the JSON output
    json pads_json = json::array();
    for (const auto& pad : pads) {
        pads_json.push_back({
            {"uid", pad.uid},
            {"coord", { {"x", pad.coord.x}, {"y", pad.coord.y} }},
            {"size", { {"width", pad.width}, {"height", pad.height} }}
        });
    }
    output["pads"] = pads_json;

    output["nets"] = generateNets();
    return output;
}


int JsonDataGenerator::getRandom(int min, int max) {
    return std::uniform_int_distribution<>(min, max)(rng);
}

Cell JsonDataGenerator::generateCell() {

    Cell cell;
    do {
        cell.width = getRandom(min_cell_width, std::min(max_cell_width, area.x));
        cell.height = getRandom(min_cell_height, std::min(max_cell_height, area.y));
        cell.coord = { getRandom(0, area.x - cell.width), getRandom(0, area.y - cell.height) };
    } while (isOverlapping(cell));

    cell.uid = "cell" + std::to_string(cell_id);
    int pin_count = getRandom(min_pin_count, max_pin_count);
    cell.pins.clear();
    std::unordered_set<Coord> used_pins;
    for (int i = 0; i < pin_count; ++i) {
        cell.pins.push_back(generatePin(cell, used_pins));
    }
    ++cell_id;
    return cell;

}


// bool JsonDataGenerator::isOverlapping(const Cell& new_cell) {
//     for (const auto& cell : cells) {
//         if (!(new_cell.coord.x + new_cell.width <= cell.coord.x ||
//               cell.coord.x + cell.width <= new_cell.coord.x ||
//               new_cell.coord.y + new_cell.height <= cell.coord.y ||
//               cell.coord.y + cell.height <= new_cell.coord.y)) {
//             return true;
//         }
//     }
//     return false;
// }
bool JsonDataGenerator::isOverlapping(const Cell& new_cell) {
    // Check overlap with other cells
    for (const auto& cell : cells) {
        if (!(new_cell.coord.x + new_cell.width <= cell.coord.x ||
              cell.coord.x + cell.width <= new_cell.coord.x ||
              new_cell.coord.y + new_cell.height <= cell.coord.y ||
              cell.coord.y + cell.height <= new_cell.coord.y)) {
            return true;
        }
    }

    // Check overlap with pads
    for (const auto& pad : pads) {
        if (!(new_cell.coord.x + new_cell.width <= pad.coord.x ||
              pad.coord.x + pad.width <= new_cell.coord.x ||
              new_cell.coord.y + new_cell.height <= pad.coord.y ||
              pad.coord.y + pad.height <= new_cell.coord.y)) {
            return true;
        }
    }

    return false;
}



json JsonDataGenerator::generateNets() {
    json nets_json = json::array();
    int net_id = 0;

    // Collect all the pins and pads in one list
    std::vector<Pin> all_pin_list;
    for (const auto& cell : cells) {
        for (const auto& pin : cell.pins) {
            all_pin_list.push_back(pin);
        }
    }

    // Add pads as well, ensuring they are part of the connection list
    for (const auto& pad : pads) {
        Pin pad_pin;
        pad_pin.uid = pad.uid;
        pad_pin.coord = adjustPadCoordInsideArea(pad);  // ← Ահա այստեղ
        all_pin_list.push_back(pad_pin);
    }

    // Used to track the connections that have been made
    std::unordered_set<std::string> used_pins;

    while (used_pins.size() < all_pin_list.size()) {
        Net net;
        net.uid = "net" + std::to_string(net_id++);
        net.weight = getRandom(0, 10); // Assign a random weight between 1 and 10

        int first_pin_index = getRandom(0, all_pin_list.size() - 1);
        int second_pin_index;

        do {
            second_pin_index = getRandom(0, all_pin_list.size() - 1);
        } while (first_pin_index == second_pin_index || used_pins.count(all_pin_list[second_pin_index].uid));

        net.connections.first = all_pin_list[first_pin_index];
        net.connections.second = all_pin_list[second_pin_index];
        // net.connections.push_back({first_pin_index].uid });
        // net.connections.push_back({ all_pin_list[second_pin_index].uid });

        used_pins.insert(all_pin_list[first_pin_index].uid);
        used_pins.insert(all_pin_list[second_pin_index].uid);

        json net_json;
        net_json["uid"] = net.uid;
        net_json["weight"] = net.weight; // Store weight in JSON
        net_json["connections"] = {
            { {"pin", net.connections.first.uid},
            {"x", net.connections.first.coord.x},
            {"y", net.connections.first.coord.y}},
            { {"pin", net.connections.second.uid},
            {"x", net.connections.second.coord.x},
            {"y", net.connections.second.coord.y}},
        };

        nets_json.push_back(net_json);
    }
    return nets_json;
}

Coord JsonDataGenerator::adjustPadCoordInsideArea(const Pad& pad) {
    Coord adjusted = pad.coord;

    if (pad.coord.x < 0) {
        adjusted.x = 0;  // դուրս է ձախից
    } else if (pad.coord.x >= area.x) {
        adjusted.x = area.x - 1;  // դուրս է աջից
    }

    if (pad.coord.y < 0) {
        adjusted.y = 0;  // դուրս է վերևից
    } else if (pad.coord.y >= area.y) {
        adjusted.y = area.y - 1;  // դուրս է ներքևից
    }

    return adjusted;
}




// Pad JsonDataGenerator::generatePad() {
//     Pad pad;
//     do {
//         // Randomly choose an edge: 0 - top, 1 - bottom, 2 - left, 3 - right
//         int edge = getRandom(0, 3);
//         int side_length = getRandom(min_pad_size, std::min(max_pad_size, area.x));
//         pad.width = side_length;
//         pad.height = side_length;  // Ensure the pad is square

//         if (edge == 0) {
//             // Place on the top edge
//             pad.coord = { getRandom(0, area.x - pad.width), 0 };
//         } else if (edge == 1) {
//             // Place on the bottom edge
//             pad.coord = { getRandom(0, area.x - pad.width), area.y - pad.height };
//         } else if (edge == 2) {
//             // Place on the left edge
//             pad.coord = { 0, getRandom(0, area.y - pad.height) };
//         } else {
//             // Place on the right edge
//             pad.coord = { area.x - pad.width, getRandom(0, area.y - pad.height) };
//         }

//     } while (isPadOverlapping(pad));

//     pad.uid = "pad" + std::to_string(pad_id);
//     ++pad_id;
//     return pad;
// }

Pad JsonDataGenerator::generatePad() {
    throw std::runtime_error("Use generateAllPads() instead of generatePad().");
}

void JsonDataGenerator::generateAllPads() {
    int size = std::min(min_pad_size, max_pad_size);
    int gap = std::max(2 * size, 30);
    int step = size + gap;

    int count_x = area.x / step;
    int count_y = area.y / step;

    // Վերևի եզրից դուրս (y = -size)
    for (int i = 0; i < count_x; ++i) {
        int x = i * step;
        pads.push_back(Pad{
            "pad" + std::to_string(pad_id++),
            Coord{ x, -size },  // դուրս վերևից
            size, size
        });
    }

    // Ստորին եզրից դուրս (y = area.y)
    for (int i = 0; i < count_x; ++i) {
        int x = i * step;
        pads.push_back(Pad{
            "pad" + std::to_string(pad_id++),
            Coord{ x, area.y },  // դուրս ներքևից
            size, size
        });
    }

    // Ձախ եզրից դուրս (x = -size)
    for (int i = 0; i < count_y; ++i) {
        int y = i * step;
        pads.push_back(Pad{
            "pad" + std::to_string(pad_id++),
            Coord{ -size, y },  // դուրս ձախից
            size, size
        });
    }

    // Աջ եզրից դուրս (x = area.x)
    for (int i = 0; i < count_y; ++i) {
        int y = i * step;
        pads.push_back(Pad{
            "pad" + std::to_string(pad_id++),
            Coord{ area.x, y },  // դուրս աջից
            size, size
        });
    }
}



// bool JsonDataGenerator::isOverlapping(const Cell& new_cell) {
//     // Check overlap with other cells
//     for (const auto& cell : cells) {
//         if (!(new_cell.coord.x + new_cell.width <= cell.coord.x ||
//               cell.coord.x + cell.width <= new_cell.coord.x ||
//               new_cell.coord.y + new_cell.height <= cell.coord.y ||
//               cell.coord.y + cell.height <= new_cell.coord.y)) {
//             return true;
//         }
//     }

//     // Check overlap with pads
//     for (const auto& pad : pads) {
//         if (!(new_cell.coord.x + new_cell.width <= pad.coord.x ||
//               pad.coord.x + pad.width <= new_cell.coord.x ||
//               new_cell.coord.y + new_cell.height <= pad.coord.y ||
//               pad.coord.y + pad.height <= new_cell.coord.y)) {
//             return true;
//         }
//     }

//     return false;
// }






// bool JsonDataGenerator::isPadOverlapping(const Pad& new_pad) {
//     for (const auto& cell : cells) {
//         if (!(new_pad.coord.x + new_pad.width <= cell.coord.x ||
//               cell.coord.x + cell.width <= new_pad.coord.x ||
//               new_pad.coord.y + new_pad.height <= cell.coord.y ||
//               cell.coord.y + cell.height <= new_pad.coord.y)) {
//             return true;
//         }
//     }
//     return false;
// }
bool JsonDataGenerator::isPadOverlapping(const Pad& new_pad) {
    // Check overlap with cells
    for (const auto& cell : cells) {
        if (!(new_pad.coord.x + new_pad.width <= cell.coord.x ||
              cell.coord.x + cell.width <= new_pad.coord.x ||
              new_pad.coord.y + new_pad.height <= cell.coord.y ||
              cell.coord.y + cell.height <= new_pad.coord.y)) {
            return true;
        }
    }

    // Check overlap with other pads
    for (const auto& pad : pads) {
        if (!(new_pad.coord.x + new_pad.width <= pad.coord.x ||
              pad.coord.x + pad.width <= new_pad.coord.x ||
              new_pad.coord.y + new_pad.height <= pad.coord.y ||
              pad.coord.y + pad.height <= new_pad.coord.y)) {
            return true;
        }
    }

    return false;
}


void JsonDataGenerator::generatePadsAndNets(QJsonObject &jsonObj, QJsonArray &cellsArray, QJsonArray &netsArray) {
    QJsonArray padsArray;

    for (int i = 0; i < cellsArray.size(); ++i) {
        QJsonObject cellObj = cellsArray[i].toObject();
        int cellX = cellObj["coord"].toObject()["x"].toInt();
        int cellY = cellObj["coord"].toObject()["y"].toInt();

        // Create a pad next to the cell (with a fixed offset)
        QJsonObject padObj;
        padObj["cellID"] = i;  // Reference to the current cell
        padObj["netID"] = i;    // Reference to the net (we can link pads and nets with the same ID for simplicity)
        padObj["size"] = QJsonObject{{"width", 10}, {"height", 10}};
        padObj["coord"] = QJsonObject{{"x", cellX + 5}, {"y", cellY + 5}};  // Pad is placed next to the cell

        padsArray.append(padObj);

        // Create a net that connects this pad to another point (e.g., a random point nearby or another pad)
        QJsonObject netObj;
        netObj["uid"] = QString("net%1").arg(i);
        netObj["weight"] = 1;

        QJsonArray connectionsArray;
        connectionsArray.append(QJsonObject{{"pin", QString("(%1,%2)").arg(cellX + 5).arg(cellY + 5)}});
        connectionsArray.append(QJsonObject{{"pin", QString("(%1,%2)").arg(cellX + 10).arg(cellY + 10)}});  // A connection to a nearby point or another pad

        netObj["connections"] = connectionsArray;

        netsArray.append(netObj);
    }

    // Assign the pads array to the JSON object
    jsonObj["pads"] = padsArray;
}





Pin JsonDataGenerator::generatePin(const Cell& cell, std::unordered_set<Coord>& used_pins) {
    Pin pin;
    do {
        int edge = getRandom(0, 3);
        if (edge == 0) {
            pin.coord = { getRandom(cell.coord.x, cell.coord.x + cell.width), cell.coord.y };
        } else if (edge == 1) {
            pin.coord = { getRandom(cell.coord.x, cell.coord.x + cell.width), cell.coord.y + cell.height };
        } else if (edge == 2) {
            pin.coord = { cell.coord.x, getRandom(cell.coord.y, cell.coord.y + cell.height) };
        } else {
            pin.coord = { cell.coord.x + cell.width, getRandom(cell.coord.y, cell.coord.y + cell.height) };
        }
    } while (used_pins.count(pin.coord));
    used_pins.insert(pin.coord);
    pin.uid = "pin" + std::to_string(pin_id);
    ++pin_id;
    return pin;
}

