#pragma once

#include "constants.hpp"

class Tank {
public:
    Constants::Position pos;
    Constants::Direction dir;
    Constants::TankType type;
    int bulletCooldown;
    bool alive;
    int hp;

    Tank(int x, int y, Constants::Direction d, Constants::TankType t);

    void Move();
    void Update();
    bool CanShoot() const;
    void OnShoot();
    Constants::Position BulletSpawn() const;
};
