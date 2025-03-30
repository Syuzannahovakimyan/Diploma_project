// #include "structurs.h"
// // #include <QFile>

// struct PlacementData {
//     std::vector<Cell> cells;
//     std::unordered_map<std::string, int> cellIndex; // Cell UID-ից index
//     std::vector<std::vector<int>> adjacencyMatrix; // Ցանցերի հիման վրա կապերը
// };

// // JSON-ից բեռնման ֆունկցիա
// PlacementData loadPlacementData(const std::string& filename) {
//     std::ifstream file(filename);
//     if (!file.is_open()) {
//         std::cerr << "Error: Cannot open file " << filename << std::endl;
//         exit(1);
//     }

//     json j;
//     file >> j;

//     PlacementData data;
//     int n = j["cells"].size();
//     data.adjacencyMatrix.resize(n, std::vector<int>(n, 0));

//     // Կարդում ենք բջիջները
//     for (size_t i = 0; i < j["cells"].size(); ++i) {
//         Cell cell;
//         cell.uid = j["cells"][i]["uid"];
//         cell.coord = { j["cells"][i]["coord"]["x"], j["cells"][i]["coord"]["y"] };
//         cell.width = j["cells"][i]["size"]["width"];
//         cell.height = j["cells"][i]["size"]["height"];

//         data.cellIndex[cell.uid] = i; // Տալիս ենք ինդեքս UID-ի համար
//         data.cells.push_back(cell);
//     }

//     // Կարդում ենք ցանցերը (nets) և կազմում adjacency matrix
//     for (const auto& net : j["nets"]) {
//         int weight = net["weight"];
//         std::vector<int> connectedIndexes;

//         for (const auto& conn : net["connections"]) {
//             std::string pinUid = conn["pin"];

//             for (size_t i = 0; i < data.cells.size(); ++i) {
//                 for (const auto& pin : data.cells[i].pins) {
//                     if (pin.uid == pinUid) {
//                         connectedIndexes.push_back(i);
//                     }
//                 }
//             }
//         }

//         // Ավելացնում ենք կապը adjacency matrix-ում
//         for (size_t i = 0; i < connectedIndexes.size(); ++i) {
//             for (size_t j = i + 1; j < connectedIndexes.size(); ++j) {
//                 int idx1 = connectedIndexes[i];
//                 int idx2 = connectedIndexes[j];
//                 data.adjacencyMatrix[idx1][idx2] += weight;
//                 data.adjacencyMatrix[idx2][idx1] += weight;
//             }
//         }
//     }

//     return data;
// }


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void applyQuadraticPlacement(PlacementData& data) {
//     int n = data.cells.size();
//     std::vector<Coord> new_positions(n);

//     for (int iter = 0; iter < 100; ++iter) { // 100 իտերացիա (կարելի է փոխել)
//         for (int i = 0; i < n; ++i) {
//             int sum_x = 0, sum_y = 0, total_weight = 0;

//             for (int j = 0; j < n; ++j) {
//                 if (i != j && data.adjacencyMatrix[i][j] > 0) {
//                     sum_x += data.adjacencyMatrix[i][j] * data.cells[j].coord.x;
//                     sum_y += data.adjacencyMatrix[i][j] * data.cells[j].coord.y;
//                     total_weight += data.adjacencyMatrix[i][j];
//                 }
//             }

//             if (total_weight > 0) {
//                 new_positions[i].x = sum_x / total_weight;
//                 new_positions[i].y = sum_y / total_weight;
//             } else {
//                 new_positions[i] = data.cells[i].coord; // Եթե կապ չունի, թող նույն տեղում մնա
//             }
//         }

//         // Թարմացնում ենք բջիջների դիրքերը
//         for (int i = 0; i < n; ++i) {
//             data.cells[i].coord = new_positions[i];
//         }
//     }
// }


// /////////////////////////////////////////



// void saveNewPlacement(const std::string& filename, const PlacementData& data) {
//     json j;
//     j["area"] = { {"width", 100}, {"height", 100} }; // Օրինակային տարածք

//     json cells_json = json::array();
//     for (const auto& cell : data.cells) {
//         json cell_json;
//         cell_json["uid"] = cell.uid;
//         cell_json["coord"] = { {"x", cell.coord.x}, {"y", cell.coord.y} };
//         cell_json["size"] = { {"width", cell.width}, {"height", cell.height} };

//         cells_json.push_back(cell_json);
//     }
//     j["cells"] = cells_json;

//     std::ofstream file(filename);
//     file << j.dump(4);
//     std::cout << "📂 New placement saved to " << filename << std::endl;
// }

// /////////////////////////////////////////
