// quadraticplacement.cpp
#include "quadraticplacement.h"
#include "matrix_compute.h"
#include <iostream>
#include <algorithm>
#include <QRect>


QuadraticPlacement::QuadraticPlacement(std::vector<Cell> cells,
                                       std::vector<Net> nets,
                                       std::vector<Pad> pads,
                                       Coord areaSize)
    : _cells(std::move(cells)), _nets(std::move(nets)), _pads(std::move(pads)), area(areaSize)
{
    size_t n = _cells.size();
    _C.resize(n, std::vector<int>(n, 0));
    _A.resize(n, std::vector<int>(n, 0));
    _Bx.resize(n, 0);
    _By.resize(n, 0);
}

void QuadraticPlacement::compute_X(std::vector<int>& x, std::vector<int>& y) {
    compute_C();
    compute_A();
    compute_Bx_By();

    Matrix_compute solver(_A, _Bx, _By);
    x = solver.print_x();
    y = solver.print_y();

    // Snap to grid layout (final layout override) avoiding pad overlaps
    // int gridSpacing = 5;
    // int gridX = 0, gridY = 0;

    std::vector<QRect> placedRects;
    int spacing = 5;  // min distance between cells (no touching)

    for (size_t i = 0; i < x.size(); ++i) {
        int w = _cells[i].width;
        int h = _cells[i].height;

        bool placed = false;

        for (int tryY = 0; tryY + h <= area.y; ++tryY) {
            for (int tryX = 0; tryX + w <= area.x; ++tryX) {
                QRect currentRect(tryX, tryY, w, h);
                QRect expandedRect = currentRect.adjusted(-spacing, -spacing, spacing, spacing);

                bool overlaps = false;

                // Check overlap with pads
                for (const auto& pad : _pads) {
                    QRect padRect(pad.coord.x, pad.coord.y, pad.width, pad.height);
                    if (expandedRect.intersects(padRect)) {
                        overlaps = true;
                        break;
                    }
                }

                // Check overlap or touching with already placed cells
                for (const auto& rect : placedRects) {
                    QRect otherExpanded = rect.adjusted(-spacing, -spacing, spacing, spacing);
                    if (expandedRect.intersects(otherExpanded)) {
                        overlaps = true;
                        break;
                    }
                }

                if (!overlaps) {
                    x[i] = tryX;
                    y[i] = tryY;
                    placedRects.push_back(currentRect);
                    placed = true;
                    break;
                }
            }

            if (placed) break;
        }

        if (!placed) {
            std::cerr << "Failed to place cell " << _cells[i].uid << " without overlap or touching.\n";
            x[i] = y[i] = -1;
        }
    }


    // for (size_t i = 0; i < x.size(); ++i) {
    //     int w = _cells[i].width;
    //     int h = _cells[i].height;

    //     bool placed = false;
    //     int attempts = 0;

    //     while (!placed && attempts < 1000) {
    //         if (gridX + w > area.x) {
    //             gridX = 0;
    //             gridY += h + gridSpacing;
    //         }

    //         if (gridY + h > area.y) {
    //             std::cerr << "Warning: Not enough area to place all cells in grid layout." << std::endl;
    //             x[i] = y[i] = -1; // invalid
    //             break;
    //         }

    //         QRect cellRect(gridX, gridY, w, h);
    //         bool overlaps = false;

    //         // Check against pads
    //         for (const auto& pad : _pads) {
    //             QRect padRect(pad.coord.x, pad.coord.y, pad.width, pad.height);
    //             if (cellRect.intersects(padRect)) {
    //                 overlaps = true;
    //                 break;
    //             }
    //         }

    //         // Check against already placed cells
    //         for (size_t j = 0; j < i; ++j) {
    //             QRect otherRect(x[j], y[j], _cells[j].width, _cells[j].height);
    //             if (cellRect.intersects(otherRect)) {
    //                 overlaps = true;
    //                 break;
    //             }
    //         }


    //         if (!overlaps) {
    //             x[i] = gridX;
    //             y[i] = gridY;
    //             gridX += w + gridSpacing;
    //             placed = true;
    //         } else {
    //             gridX += gridSpacing;
    //             attempts++;
    //         }
    //     }

    //     if (attempts >= 1000) {
    //         std::cerr << "Failed to place cell " << _cells[i].uid << " without overlapping a pad.\n";
    //         x[i] = y[i] = -1;
    //     }
    // }
}

void QuadraticPlacement::compute_C() {
    for (size_t i = 0; i < _cells.size(); ++i) {
        for (size_t j = i; j < _cells.size(); ++j) {
            if (Net net; cells_is_connected(_cells[i], _cells[j], net)) {
                _C[i][j] = _C[j][i] = net.weight;
            }
        }
    }
}

void QuadraticPlacement::compute_A() {
    for (size_t i = 0; i < _cells.size(); ++i) {
        for (size_t j = i; j < _cells.size(); ++j) {
            if (i == j) {
                auto [weight, _] = get_pad_weight_and_coord(_cells[i]);
                _A[i][j] = sum(i) + weight;
            } else {
                _A[i][j] = _A[j][i] = -_C[i][j];
            }
        }
    }
}

void QuadraticPlacement::compute_Bx_By() {
    for (size_t i = 0; i < _cells.size(); ++i) {
        auto [weight, coord] = get_pad_weight_and_coord(_cells[i]);
        _Bx[i] = weight * coord.x;
        _By[i] = weight * coord.y;
    }
}

int QuadraticPlacement::sum(size_t i) {
    int total = 0;
    for (size_t j = 0; j < _C[i].size(); ++j) {
        total += _C[i][j];
    }
    return total;
}

std::pair<int, Coord> QuadraticPlacement::get_pad_weight_and_coord(const Cell& cell) {
    for (const auto& pad : _pads) {
        for (const auto& net : _nets) {
            if ((net.connections.first.coord == pad.coord ||
                 net.connections.second.coord == pad.coord) &&
                cell.net_is_exist(net)) {
                return { net.weight, pad.coord };
            }
        }
    }
    return {0, {0, 0}};
}

bool QuadraticPlacement::cells_is_connected(const Cell& cell1, const Cell& cell2, Net& net) const {
    for (const auto& n : _nets) {
        if (cell1.net_is_exist(n) && cell2.net_is_exist(n)) {
            net = n;
            return true;
        }
    }
    return false;
}
