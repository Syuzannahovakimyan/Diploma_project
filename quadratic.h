// // #ifndef quadratic_h
// // #define quadratic_h

// // #include "structurs.h"
// // #include <vector>
// // #include <QJsonObject>
// // #include <nlohmann/json.hpp>
// // #include <unordered_map>

// // using json = nlohmann::json;

// // class QuadraticPlacement {
// // public:
// //     QuadraticPlacement(const QJsonObject &jsonObj);
// //     void applyPlacement();
// //     QJsonObject getUpdatedJson() const;

// // private:
// //     void computeNewPositions();
// //     double calculateWeightedCenterX(const Cell &cell);
// //     double calculateWeightedCenterY(const Cell &cell);
// //     void updatePins(Cell &cell);

// //     int areaWidth;
// //     int areaHeight;
// //     std::vector<Cell> cells;
// //     std::vector<Net> nets;
// //     std::unordered_map<std::string, Cell*> cellMap;
// // };
// // #endif //quadratic_h


// #include <QJsonObject>
// #include <QJsonArray>
// #include <QJsonDocument>
// #include <iostream>
// #include <cmath>

// class QuadraticPlacement {
// private:
//     QJsonObject jsonObj; // JSON օբյեկտը, որը վերցվում է արտաքինից

// public:
//     // Կառուցապատող՝ ընդունում է JSON օբյեկտ
//     QuadraticPlacement(const QJsonObject& obj) : jsonObj(obj) {}

//     // Հիմնական մեթոդը՝ հրապարակումը
//     void applyPlacement() {
//         std::cout << "Applying Quadratic Placement..." << std::endl;

//         // Ստուգենք, թե արդյոք "cells" մասն է JSON-ում
//         if (jsonObj.contains("cells")) {
//             QJsonArray cells = jsonObj["cells"].toArray();

//             // Նախևառաջ պետք է տվյալները դարձնենք օգտագործելի
//             for (int i = 0; i < cells.size(); ++i) {
//                 QJsonObject cell = cells[i].toObject();
//                 int x = cell["x"].toInt();
//                 int y = cell["y"].toInt();
//                 int weight = cell["weight"].toInt();
//                 std::cout << "Cell " << i + 1 << " at (" << x << "," << y << ") with weight " << weight << std::endl;
//             }

//             // Այժմ մենք կարող ենք աշխատել Quadratic Placement ալգորիթմի սկզբունքով:
//             applyQuadraticPlacement(cells); // Ապահովում ենք, որ մեր բջիջները տեղադրվեն ճիշտ
//         }
//     }

//     // Ալգորիթմը՝ բջիջները տեղադրելու համար
//     void applyQuadraticPlacement(QJsonArray& cells) {
//         // Թարգմանում ենք բջիջների հարաբերական տեղադրման սկզբունքները
//         int totalCells = cells.size();

//         // Հաշվարկում ենք ցանցի գոտիների չափերը (հասարակապես միջին արժեքներ, որ կարող ենք օգտագործել)
//         int gridSize = static_cast<int>(std::sqrt(totalCells)) + 1;

//         // Այն բջիջները, որոնք պահանջում են տեղադրման փոփոխություն
//         for (int i = 0; i < totalCells; ++i) {
//             QJsonObject cell = cells[i].toObject();

//             // Թույլատրված տեղադրումների համար՝ հաշվենք x, y նոր դիրքեր
//             int x = (i % gridSize) * 10; // Բջիջը ցանցում
//             int y = (i / gridSize) * 10;

//             // Վերադարձնենք նոր դիրքեր JSON օբյեկտում
//             cell["x"] = x;
//             cell["y"] = y;
//             cells[i] = cell;
//         }
//     }

//     // Փոփոխված JSON վերադարձնելու մեթոդը
//     QJsonObject getUpdatedJson() {
//         // Վերադարձնում ենք նոր JSON, որտեղ փոփոխված դիրքերը
//         QJsonObject updatedObj = jsonObj; // Նոր объект՝ արդեն փոփոխված տեղադրումներ

//         QJsonArray updatedCells;
//         for (int i = 0; i < jsonObj["cells"].toArray().size(); ++i) {
//             QJsonObject cell = jsonObj["cells"].toArray()[i].toObject();
//             updatedCells.append(cell);
//         }

//         updatedObj.insert("cells", updatedCells);

//         return updatedObj;
//     }
// };
