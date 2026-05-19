#include "tank.hpp"

Tank::Tank(int x, int y, Constants::Direction d, Constants::TankType t)
    : pos{x, y}, dir(d), type(t), bulletCooldown(0), alive(true), hp(1) {}

void Tank::Move() {
    switch (dir) {
    case Constants::Direction::UP:    pos.y--; break;
    case Constants::Direction::DOWN:  pos.y++; break;
    case Constants::Direction::LEFT:  pos.x--; break;
    case Constants::Direction::RIGHT: pos.x++; break;
    }
}

void Tank::Update() {
    if (bulletCooldown > 0) bulletCooldown--;
}

bool Tank::CanShoot() const {
    return alive && bulletCooldown == 0;
}

void Tank::OnShoot() {
    bulletCooldown = Constants::BULLET_COOLDOWN_TICKS;
}

Constants::Position Tank::BulletSpawn() const {
    switch (dir) {
    case Constants::Direction::UP:    return {pos.x, pos.y - 1};
    case Constants::Direction::DOWN:  return {pos.x, pos.y + 1};
    case Constants::Direction::LEFT:  return {pos.x - 1, pos.y};
    case Constants::Direction::RIGHT: return {pos.x + 1, pos.y};
    }
    return pos;
}
