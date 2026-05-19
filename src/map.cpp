#include "map.hpp"

Map::Map() : basePos{12, 24} {
    Clear();
    LoadLevel(1);
}

void Map::Clear() {
    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        for (int x = 0; x < Constants::MAP_WIDTH; x++) {
            grid[y][x] = Constants::CellType::EMPTY;
        }
    }
}

void Map::LoadLevel(int level) {
    Clear();
    spawnPoints.clear();

    // Enemy spawn points (top area)
    spawnPoints.push_back({1, 0});
    spawnPoints.push_back({8, 0});
    spawnPoints.push_back({15, 0});
    spawnPoints.push_back({22, 0});

    // Place base
    basePos = {12, 24};
    grid[basePos.y][basePos.x] = Constants::CellType::BASE;

    // Brick walls around base
    grid[23][11] = Constants::CellType::BRICK;
    grid[23][12] = Constants::CellType::BRICK;
    grid[23][13] = Constants::CellType::BRICK;
    grid[24][11] = Constants::CellType::BRICK;
    grid[24][13] = Constants::CellType::BRICK;
    grid[25][11] = Constants::CellType::BRICK;
    grid[25][12] = Constants::CellType::BRICK;
    grid[25][13] = Constants::CellType::BRICK;

    // Level-specific layouts
    if (level % 3 == 1) {
        // Level 1: Symmetric brick clusters
        for (int row : {3, 7, 11, 15, 19}) {
            for (int x = 2; x <= 10; x += 2) {
                grid[row][x] = Constants::CellType::BRICK;
                grid[row][25 - x] = Constants::CellType::BRICK;
            }
        }
        for (int x : {1, 5, 9, 13, 16, 20, 24}) {
            grid[5][x] = Constants::CellType::BRICK;
            grid[9][x] = Constants::CellType::BRICK;
            grid[13][x] = Constants::CellType::BRICK;
            grid[17][x] = Constants::CellType::BRICK;
            grid[21][x] = Constants::CellType::BRICK;
        }
        // Steel pillars
        grid[6][6] = Constants::CellType::STEEL;
        grid[6][19] = Constants::CellType::STEEL;
        grid[14][6] = Constants::CellType::STEEL;
        grid[14][19] = Constants::CellType::STEEL;
    } else if (level % 3 == 2) {
        // Level 2: Corridor maze
        for (int y = 4; y <= 20; y += 4) {
            for (int x = 0; x < 26; x++) {
                if (x != 5 && x != 8 && x != 17 && x != 20) {
                    grid[y][x] = Constants::CellType::BRICK;
                }
            }
        }
        for (int x = 4; x <= 21; x += 8) {
            for (int y = 6; y <= 18; y++) {
                grid[y][x] = Constants::CellType::BRICK;
            }
        }
        // Steel walls
        grid[2][12] = Constants::CellType::STEEL;
        grid[2][13] = Constants::CellType::STEEL;
        grid[10][0] = Constants::CellType::STEEL;
        grid[10][1] = Constants::CellType::STEEL;
        grid[10][24] = Constants::CellType::STEEL;
        grid[10][25] = Constants::CellType::STEEL;
        grid[18][12] = Constants::CellType::STEEL;
        grid[18][13] = Constants::CellType::STEEL;
    } else {
        // Level 3: Open arena with scattered cover
        for (int y = 3; y <= 21; y += 3) {
            for (int x = 3; x <= 22; x += 6) {
                grid[y][x] = Constants::CellType::BRICK;
                grid[y][x + 1] = Constants::CellType::BRICK;
            }
        }
        // Central steel cross
        for (int y = 5; y <= 19; y++) {
            grid[y][12] = Constants::CellType::STEEL;
            grid[y][13] = Constants::CellType::STEEL;
        }
        for (int x = 5; x <= 20; x++) {
            grid[12][x] = Constants::CellType::STEEL;
        }
        // Gaps in the cross
        grid[9][12] = Constants::CellType::EMPTY;
        grid[9][13] = Constants::CellType::EMPTY;
        grid[15][12] = Constants::CellType::EMPTY;
        grid[15][13] = Constants::CellType::EMPTY;
        grid[12][9] = Constants::CellType::EMPTY;
        grid[12][10] = Constants::CellType::EMPTY;
        grid[12][15] = Constants::CellType::EMPTY;
        grid[12][16] = Constants::CellType::EMPTY;
    }
}

Constants::CellType Map::GetCell(int x, int y) const {
    if (!IsInBounds(x, y)) return Constants::CellType::STEEL;
    return grid[y][x];
}

void Map::SetCell(int x, int y, Constants::CellType type) {
    if (IsInBounds(x, y)) grid[y][x] = type;
}

bool Map::IsSolid(int x, int y) const {
    auto cell = GetCell(x, y);
    return cell == Constants::CellType::BRICK || cell == Constants::CellType::STEEL;
}

bool Map::IsInBounds(int x, int y) const {
    return x >= 0 && x < Constants::MAP_WIDTH && y >= 0 && y < Constants::MAP_HEIGHT;
}

Constants::Position Map::BasePosition() const {
    return basePos;
}

std::vector<Constants::Position> Map::EnemySpawns() const {
    return spawnPoints;
}
