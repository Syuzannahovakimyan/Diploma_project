#include "generator.h"

JsonDataGenerator::JsonDataGenerator(Coord area, int cell_count, int min_cell_width, int max_cell_width,
                                     int min_cell_height, int max_cell_height, int min_pin_count, int max_pin_count)
    : area(area), cell_count(cell_count), min_cell_width(min_cell_width), max_cell_width(max_cell_width),
    min_cell_height(min_cell_height), max_cell_height(max_cell_height), min_pin_count(min_pin_count), max_pin_count(max_pin_count) {
    rng.seed(std::random_device{}());
}

json JsonDataGenerator::generate() {
    json output;
    output["area"] = { {"width", area.x}, {"height", area.y} };

    cells.clear();
    all_pins.clear();

    while (cells.size() < static_cast<size_t>(cell_count)) {
        Cell new_cell = generateCell();
        if (!isOverlapping(new_cell)) {
            cells.push_back(new_cell);
            for (const auto& pin : new_cell.pins) {
                all_pins[pin.uid] = pin;
            }
        }
    }

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

    cell.uid = "cell(" + std::to_string(cell.coord.x) + ", " + std::to_string(cell.coord.y) + ")";

    int pin_count = getRandom(min_pin_count, max_pin_count);
    cell.pins.clear();
    std::unordered_set<Coord> used_pins;
    for (int i = 0; i < pin_count; ++i) {
        cell.pins.push_back(generatePin(cell, used_pins));
    }
    return cell;
}

bool JsonDataGenerator::isOverlapping(const Cell& new_cell) {
    for (const auto& cell : cells) {
        if (!(new_cell.coord.x + new_cell.width <= cell.coord.x ||
              cell.coord.x + cell.width <= new_cell.coord.x ||
              new_cell.coord.y + new_cell.height <= cell.coord.y ||
              cell.coord.y + cell.height <= new_cell.coord.y)) {
            return true;
        }
    }
    return false;
}

json JsonDataGenerator::generateNets() {
    json nets_json = json::array();
    int net_id = 1;

    std::vector<Pin> all_pin_list;
    for (const auto& cell : cells) {
        for (const auto& pin : cell.pins) {
            all_pin_list.push_back(pin);
        }
    }

    std::unordered_set<std::string> used_pins;
    while (used_pins.size() < all_pin_list.size()) {
        Net net;
        net.uid = "net" + std::to_string(net_id++);
        net.weight = getRandom(1, 10); // Assign a random weight between 1 and 10

        int first_pin_index = getRandom(0, all_pin_list.size() - 1);
        int second_pin_index;

        do {
            second_pin_index = getRandom(0, all_pin_list.size() - 1);
        } while (first_pin_index == second_pin_index || used_pins.count(all_pin_list[second_pin_index].uid));

        net.connections.push_back({ all_pin_list[first_pin_index].uid });
        net.connections.push_back({ all_pin_list[second_pin_index].uid });

        used_pins.insert(all_pin_list[first_pin_index].uid);
        used_pins.insert(all_pin_list[second_pin_index].uid);

        json net_json;
        net_json["uid"] = net.uid;
        net_json["weight"] = net.weight; // Store weight in JSON
        net_json["connections"] = {
            { {"pin", net.connections[0].pin} },
            { {"pin", net.connections[1].pin} }
        };

        nets_json.push_back(net_json);
    }
    return nets_json;
}


// json JsonDataGenerator::generateNets() {
//     json nets_json = json::array();
//     int net_id = 1;

//     std::vector<Pin> all_pin_list;
//     for (const auto& cell : cells) {
//         for (const auto& pin : cell.pins) {
//             all_pin_list.push_back(pin);
//         }
//     }

//     std::unordered_set<std::string> used_pins;
//     while (used_pins.size() < all_pin_list.size()) {
//         Net net;
//         net.uid = "net" + std::to_string(net_id++);

//         // Վերջին փինն արդեն միացված է մյուսներին
//         int first_pin_index = getRandom(0, all_pin_list.size() - 1);
//         int second_pin_index;

//         // Սահմանափակենք, որպեսզի մեկ նեթը չմիացնի երկու անգամ նույն փիները
//         do {
//             second_pin_index = getRandom(0, all_pin_list.size() - 1);
//         } while (first_pin_index == second_pin_index || used_pins.count(all_pin_list[second_pin_index].uid));

//         net.connections.push_back({ all_pin_list[first_pin_index].uid });
//         net.connections.push_back({ all_pin_list[second_pin_index].uid });

//         used_pins.insert(all_pin_list[first_pin_index].uid);
//         used_pins.insert(all_pin_list[second_pin_index].uid);

//         json net_json;
//         net_json["uid"] = net.uid;
//         net_json["connections"] = {
//             { {"pin", net.connections[0].pin} },
//             { {"pin", net.connections[1].pin} }
//         };
//         nets_json.push_back(net_json);
//     }
//     return nets_json;
// }


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
    pin.uid = "pin(" + std::to_string(pin.coord.x) + ", " + std::to_string(pin.coord.y) + ")";
    return pin;
}

