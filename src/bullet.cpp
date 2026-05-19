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
    case Constants::Direction::UP:    pos.y -= Constants::BULLET_SPEED; break;
    case Constants::Direction::DOWN:  pos.y += Constants::BULLET_SPEED; break;
    case Constants::Direction::LEFT:  pos.x -= Constants::BULLET_SPEED; break;
    case Constants::Direction::RIGHT: pos.x += Constants::BULLET_SPEED; break;
    }
}

void Bullet::Deactivate() {
    active = false;
}
