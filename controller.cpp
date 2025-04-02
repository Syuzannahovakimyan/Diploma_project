#include "controller.h"

Controller::Controller() {

    Coord area = {500, 500};
    JsonDataGenerator generator(area,1000, 1, 5, 1, 5, 1, 3, 5, 5, 5);
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


    // JsonDataReader reader("../output.json");
    // reader.readData();
    // std::vector<Cell>  cells = reader.getCells();
    // std::vector<Net>   nets = reader.getNets();
    // std::vector<Pad>   pads = reader.getPads();


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


    ////////////////////////////////////////////////


    // QuadraticPlacement placer(cells, nets, pads,area);

    // // Հաշվենք X ու Y կոորդինատները
    // std::vector<int> x, y;
    // placer.compute_X(x, y);

    //Տպում ենք արդյունքները
    // std::cout << "Placement results:\n";
    // for (size_t i = 0; i < cells.size(); ++i) {
    //     std::cout << "Cell " << cells[i].uid << " -> ("
    //               << x[i] << ", " << y[i] << ")\n";
    // }

    // JsonPlacementWriter writer("/home/sh/diplom/build/output.json", "/home/sh/diplom/build/updated_output.json");
    // writer.writeUpdatedJson(cells, x, y);
    // try {
    //     JsonPlacementWriter writer("/home/sh/diplom/build/output.json", "/home/sh/diplom/build/updated_output.json");
    //     writer.writeUpdatedJson(cells, x, y);
    // } catch (const std::exception& e) {
    //     std::cerr << "Error during JSON writing: " << e.what() << std::endl;
    // }


}
