#pragma once

#include "constants.hpp"
#include <array>
#include <vector>

class Map {
public:
    std::array<std::array<Constants::CellType, Constants::MAP_WIDTH>, Constants::MAP_HEIGHT> grid;

    Map();
    void LoadLevel(int level);
    Constants::CellType GetCell(int x, int y) const;
    void SetCell(int x, int y, Constants::CellType type);
    bool IsSolid(int x, int y) const;
    bool IsInBounds(int x, int y) const;
    Constants::Position BasePosition() const;
    std::vector<Constants::Position> EnemySpawns() const;

private:
    Constants::Position basePos;
    std::vector<Constants::Position> spawnPoints;
    void Clear();
};
