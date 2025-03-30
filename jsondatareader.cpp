#include "jsondatareader.h"

JsonDataReader::JsonDataReader(const std::string& filename) : filename(filename) {}

void JsonDataReader::readData() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return ;
    }

    json j;
    file >> j;

    // Կարդում ենք տարածքի չափսերը
    int area_width = j["area"]["width"];
    int area_height = j["area"]["height"];
    std::cout << "📏 Area Size: " << area_width << "x" << area_height << std::endl;

    // Կարդում ենք բջիջները
    cells.clear();
    std::cout << "\n📦 Cells:\n";
    for (const auto& cell_json : j["cells"]) {
        Cell cell;
        cell.uid = cell_json["uid"];
        cell.coord = { cell_json["coord"]["x"], cell_json["coord"]["y"] };
        cell.width = cell_json["size"]["width"];
        cell.height = cell_json["size"]["height"];

        std::cout << "  🏠 " << cell.uid <<" size: " << cell.width << "x" << cell.height << std::endl;

        // Կարդում ենք փիները
        for (const auto& pin_json : cell_json["pins"]) {
            Pin pin;
            pin.uid = pin_json["uid"];
            pin.coord = { pin_json["coord"]["x"], pin_json["coord"]["y"] };
            cell.pins.push_back(pin);

            std::cout << "     📍 Pin: " << pin.uid  << std::endl;
        }

        cells.push_back(cell);
    }
    std::cout << "Total Cells: " << cells.size() << "\n" << std::endl;

    // Կարդում ենք ցանցերը
    nets.clear();
    std::cout << "🔗 Nets:\n";
    for (const auto& net_json : j["nets"]) {
        Net net;
        net.uid = net_json["uid"];
        net.weight = net_json["weight"];

        std::cout << "  🌐 " << net.uid << " (weight: " << net.weight << ") connects:\n";

        net.connections.first.uid = net_json["connections"][0]["pin"];
        net.connections.first.coord.x = net_json["connections"][0]["x"];
        net.connections.first.coord.y = net_json["connections"][0]["y"];

        net.connections.second.uid = net_json["connections"][1]["pin"];
        net.connections.second.coord.x = net_json["connections"][1]["x"];
        net.connections.second.coord.y = net_json["connections"][1]["y"];

        std::cout << "     ➡️ " << net.connections.first.uid << std::endl;
        std::cout << "     ➡️ " << net.connections.second.uid << std::endl;
        // for (const auto& conn : net_json["connections"]) {

        //     NetConnection net_conn;
        //     net_conn.pin = conn["pin"];
        //     net.connections.push_back(net_conn);
            // std::cout << "     ➡️ " << net_conn.pin << std::endl;
        // }

        nets.push_back(net);
    }
    std::cout << "Total Nets: " << nets.size() << std::endl;


    // Կարդում ենք պադերը
    pads.clear();
    std::cout << "\n📍 Pads:\n";
    for (const auto& pad_json : j["pads"]) {
        Pad pad;
        pad.uid = pad_json["uid"];
        pad.coord = { pad_json["coord"]["x"], pad_json["coord"]["y"] };
        pad.width = pad_json["size"]["width"];
        pad.height = pad_json["size"]["height"];

        std::cout << "  🏗️ " << pad.uid << " at (" << pad.coord.x << ", " << pad.coord.y << ")"
                  << " size: " << pad.width << "x" << pad.height << std::endl;

        pads.emplace_back(std::move(pad));
    }
    std::cout << "Total Pads: " << pads.size() << std::endl;
}


