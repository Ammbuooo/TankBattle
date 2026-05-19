#pragma once

namespace Constants {

constexpr int MAP_WIDTH = 26;
constexpr int MAP_HEIGHT = 26;

constexpr int MAX_ENEMIES_ON_SCREEN = 5;
constexpr int ENEMIES_PER_LEVEL = 20;
constexpr int PLAYER_LIVES = 3;

constexpr int TICK_INTERVAL_MS = 50;
constexpr int BULLET_COOLDOWN_TICKS = 8;
constexpr int ENEMY_SHOOT_COOLDOWN = 40;
constexpr int ENEMY_SPAWN_INTERVAL = 60;

constexpr int BULLET_SPEED = 2;
constexpr int TANK_SPEED = 1;

enum class Direction { UP, DOWN, LEFT, RIGHT, NONE };

enum class CellType { EMPTY, BRICK, STEEL, BASE };

enum class TankType { PLAYER, ENEMY };

struct Position {
    int x, y;
    bool operator==(const Position& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Position& o) const { return !(*this == o); }
};

} // namespace Constants
