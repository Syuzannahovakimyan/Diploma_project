
#include "mincutplacement.h"
#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include <algorithm>
#include <iostream>

MinCutPlacement::MinCutPlacement(std::vector<Cell>& cells,
                                 const std::vector<Net>& nets,
                                 const std::vector<Pad>& pads,
                                 const Coord& areaSize)
    : cells(cells), nets(nets), pads(pads), areaSize(areaSize) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void MinCutPlacement::run(std::vector<int>& x, std::vector<int>& y) {
    std::vector<Cell*> left, right;
    partition(left, right);

    int spacing = 5;
    int cellSpacingX = 10;
    int startXLeft = spacing;
    int startXRight = areaSize.x / 2 + spacing;

    auto placeCells = [&](std::vector<Cell*>& cells, int startX, const std::string& side) {
        int currentX = startX;
        int currentY = spacing;

        int maxX = currentX;
        int maxY = 0;

        for (Cell* c : cells) {
            // Եթե չի տեղավորվում՝ նոր սյունակ սկսենք
            if (currentY + c->height > areaSize.y - spacing) {
                currentX += c->width + cellSpacingX;
                currentY = spacing;
            }

            // Ստուգենք արդյոք x չի անցնում արեայի սահմանը
            if (currentX + c->width > (side == "left" ? areaSize.x / 2 : areaSize.x)) {
                std::cerr << "Error: Not enough area in " << side << " region to place all cells!" << std::endl;
                return;
            }

            c->coord = {currentX, currentY};
            x.push_back(c->coord.x);
            y.push_back(c->coord.y);

            currentY += c->height + spacing;
        }
    };

    placeCells(left, startXLeft, "left");
    placeCells(right, startXRight, "right");
}




void MinCutPlacement::partition(std::vector<Cell*>& left, std::vector<Cell*>& right) {
    std::vector<Cell*> allCells;
    for (auto& c : cells) {
        allCells.push_back(&c);
    }

    std::random_shuffle(allCells.begin(), allCells.end());

    size_t half = allCells.size() / 2;
    left.assign(allCells.begin(), allCells.begin() + half);
    right.assign(allCells.begin() + half, allCells.end());
}

int MinCutPlacement::computeCutSize(const std::vector<Cell*>& left, const std::vector<Cell*>& right) {
    std::unordered_set<std::string> leftSet, rightSet;
    for (auto* c : left) leftSet.insert(c->uid);
    for (auto* c : right) rightSet.insert(c->uid);

    int cutSize = 0;
    for (const Net& net : nets) {
        std::string uid1 = net.connections.first.uid;
        std::string uid2 = net.connections.second.uid;

        bool inLeft = false, inRight = false;

        for (const Cell* c : left) {
            for (const Pin& p : c->pins) {
                if (p.uid == uid1 || p.uid == uid2) {
                    inLeft = true;
                    break;
                }
            }
        }

        for (const Cell* c : right) {
            for (const Pin& p : c->pins) {
                if (p.uid == uid1 || p.uid == uid2) {
                    inRight = true;
                    break;
                }
            }
        }

        if (inLeft && inRight) {
            cutSize += net.weight;
        }
    }

    return cutSize;
}
