#include "controller.h"



Controller::Controller() {

    Coord area = {500, 500};
    JsonDataGenerator generator(area,400, 1, 30, 1, 30, 1, 2, 20, 10, 10);
    json jsonData = generator.generate();
    std::ofstream file("../output.json");
    if (file.is_open()) {
        file << jsonData.dump(4);
        file.close();
        std::cout << "JSON data generated successfully and saved to output.json!" << std::endl;
    } else {
        std::cerr << "Failed to open output.json for writing!" << std::endl;
    }

    ////////////////////////////////////////////

    // Visualizer window;
    // window.show();
    v = new Visualizer;
    v->show();
    qDebug() << "Visualizer window shown!";


    /////////////////////////////////////////////


    JsonDataReader reader("../output.json");
    reader.readData();
    std::vector<Cell>  cells = reader.getCells();
    std::vector<Net>   nets = reader.getNets();
    std::vector<Pad>   pads = reader.getPads();


    LineSearchRouting router(cells, pads);

    for (const auto& net : nets) {
        Coord start = getAdjustedPinPosition(net.connections.first, cells, pads);
        Coord end = getAdjustedPinPosition(net.connections.second, cells, pads);

        auto path = router.route(start, end);

        if (!path.empty()) {
            std::cout << "✅ Net [" << net.uid << "] routed successfully:\n";
            for (const auto& point : path) {
                std::cout << "(" << point.x << ", " << point.y << ") ";
            }
            std::cout << "\n";
        } else {
            std::cout << "❌ Net [" << net.uid << "] routing failed.\n";
        }
    }



    // for (const Cell& cell : cells) {
    //     std::cout << "  🏠 " << cell.uid << " at (" << cell.coord.x << ", " << cell.coord.y
    //               << "), size: " << cell.width << "x" << cell.height << std::endl;

    //     // Տպում ենք փիները
    //     for (const Pin& pin : cell.pins) {
    //         std::cout << "     📍 Pin: " << pin.uid << " at (" << pin.coord.x << ", " << pin.coord.y << ")" << std::endl;
    //     }
    // }

    // // for (const Net& net : nets) {
    // //     std::cout << "  🌐 " << net.uid << " (weight: " << net.weight << ") connects:\n";

    // //     for (const NetConnection& conn : net.connections) {
    // //         std::cout << "     ➡️ " << conn.pin << std::endl;
    // //     }
    // // }
}

Coord Controller::getAdjustedPinPosition(const Pin& pin, const std::vector<Cell>& cells, const std::vector<Pad>& pads) {
    for (const auto& cell : cells) {
        for (size_t i = 0; i < cell.pins.size(); ++i) {
            if (cell.pins[i].uid == pin.uid) {
                // Դիրքը cell-ի սահմանի վրա, կախված pin-ի ինդեքսից
                return Coord{
                    cell.coord.x + ((i % 2 == 0) ? 0 : cell.width),  // x-ը ձախ կամ աջ սահմանին
                    cell.coord.y + ((i < 2) ? 0 : cell.height)       // y-ը վերևի կամ ներքևի սահմանին
                };
            }
        }
    }

    for (const auto& pad : pads) {
        if (pad.uid == pin.uid) {
            // Pad-ի կենտրոնը վերցնենք
            return Coord{pad.coord.x + pad.width / 2, pad.coord.y + pad.height / 2};
        }
    }

    return pin.coord; // default fallback
}


