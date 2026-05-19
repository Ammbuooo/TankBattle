#pragma once

#include "constants.hpp"
#include "tank.hpp"
#include "bullet.hpp"
#include "map.hpp"
#include <vector>
#include <memory>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

enum class GameState { PLAYING, PAUSED, WIN, LOSE };

class Game {
public:
    Game();

    void Update();
    void HandleInput(const ftxui::Event& event);
    ftxui::Element Render() const;

    bool Running() const;
    int Score() const;
    int Level() const;
    int Lives() const;
    GameState GetState() const;

private:
    void SpawnEnemy();
    void ProcessBullets();
    void ProcessEnemyAI();
    void CheckCollisions();
    void CheckBaseDestroyed();
    void NextLevel();
    void ResetPlayer();
    bool TankCollides(int x, int y, int ignoreIdx = -1) const;
    void MovePlayer(Constants::Direction d);
    void PlayerShoot();

    Map map;
    std::unique_ptr<Tank> player;
    std::vector<Tank> enemies;
    std::vector<Bullet> bullets;

    int score;
    int level;
    int lives;
    int enemySpawnTimer;
    int enemiesSpawned;
    int enemiesKilled;
    GameState state;
};
