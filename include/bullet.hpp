#pragma once

#include "constants.hpp"

class Bullet {
public:
    Constants::Position pos;
    Constants::Direction dir;
    bool active;
    bool fromPlayer;

    Bullet();
    void Fire(Constants::Position start, Constants::Direction d, bool playerBullet);
    void Move();
    void Deactivate();
};
