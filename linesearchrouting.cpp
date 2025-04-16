#include "linesearchrouting.h"
#include <algorithm>

LineSearchRouting::LineSearchRouting(const std::vector<Cell>& cells, const std::vector<Pad>& pads) {
    // Բոլոր cells և pads-ը վերածում ենք խոչընդոտների
    for (const auto& cell : cells) {
        obstacles.push_back({cell.coord.x, cell.coord.y, cell.width, cell.height});
    }
    for (const auto& pad : pads) {
        obstacles.push_back({pad.coord.x, pad.coord.y, pad.width, pad.height});
    }
}

bool LineSearchRouting::isFree(const Coord& start, const Coord& end) {
    for (const auto& obs : obstacles) {
        if (intersectLineRect(start, end, obs)) {
            return false;
        }
    }
    return true;
}

// Ստուգում է ուղիղ գծի հատումը ուղղանկյան հետ
bool LineSearchRouting::intersectLineRect(const Coord& start, const Coord& end, const Rect& rect) {
    // Միայն ուղղահայաց կամ հորիզոնական գծերի դեպքում
    if (start.x == end.x) { // Ուղղահայաց գիծ
        int y_min = std::min(start.y, end.y);
        int y_max = std::max(start.y, end.y);
        return (start.x >= rect.x && start.x < rect.x + rect.width) &&
               !(y_max < rect.y || y_min > rect.y + rect.height);
    } else if (start.y == end.y) { // Հորիզոնական գիծ
        int x_min = std::min(start.x, end.x);
        int x_max = std::max(start.x, end.x);
        return (start.y >= rect.y && start.y <= rect.y + rect.height) &&
               !(x_max < rect.x || x_min > rect.x + rect.width);
    }

    return true; // Եթե թեք գիծ է, ապա համարվում է հատում կա
}

std::vector<Coord> LineSearchRouting::route(const Coord& start, const Coord& end) {
    std::vector<Coord> path;

    if (isFree(start, end)) {
        path.push_back(start);
        path.push_back(end);
        return path;
    }

    // Փորձում ենք Manhattan style՝ առաջինը հորիզոնական հետո ուղղահայաց
    Coord mid1 = {start.x, end.y};
    Coord mid2 = {end.x, start.y};

    if (isFree(start, mid1) && isFree(mid1, end)) {
        path = {start, mid1, end};
        return path;
    }

    if (isFree(start, mid2) && isFree(mid2, end)) {
        path = {start, mid2, end};
        return path;
    }

    // Եթե չի ստացվում, վերադարձնում ենք դատարկ
    return {};
}
// Pin-ի ճիշտ դիրքը հաշվելու ֆունկցիա
