#ifndef LINE_SEARCH_ROUTING_H
#define LINE_SEARCH_ROUTING_H

#include "structurs.h"
#include <vector>

struct Rect {
    int x, y, width, height;

    bool contains(const Coord& p) const {
        return p.x >= x && p.x <= (x + width) &&
               p.y >= y && p.y <= (y + height);
    }
};

class LineSearchRouting {
public:
    LineSearchRouting(const std::vector<Cell>& cells, const std::vector<Pad>& pads);
    std::vector<Coord> route(const Coord& start, const Coord& end);

private:
    std::vector<Rect> obstacles;

    bool isFree(const Coord& start, const Coord& end);
    bool intersectLineRect(const Coord& start, const Coord& end, const Rect& rect);
};

#endif // LINE_SEARCH_ROUTING_H
