#include "../include/bullet.hpp"

Bullet::Bullet() : pos{0, 0}, dir(Constants::Direction::UP), active(false), fromPlayer(false) {}

void Bullet::Fire(Constants::Position start, Constants::Direction d, bool playerBullet) {
    pos = start;
    dir = d;
    active = true;
    fromPlayer = playerBullet;
}

void Bullet::Move() {
    switch (dir) {
    case Constants::Direction::UP:    pos.y--; break;
    case Constants::Direction::DOWN:  pos.y++; break;
    case Constants::Direction::LEFT:  pos.x--; break;
    case Constants::Direction::RIGHT: pos.x++; break;
    default: break;
    }
}

void Bullet::Deactivate() {
    active = false;
}
