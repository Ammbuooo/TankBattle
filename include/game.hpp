#pragma once

#include "constants.hpp"
#include "tank.hpp"
#include "bullet.hpp"
#include "map.hpp"
#include <vector>
#include <memory>

enum class GameState { PLAYING, PAUSED, WIN, LOSE };

struct Explosion {
    Constants::Position pos;
    int timer; // ticks remaining
};

class Game {
public:
    Game();

    void Update();
    void SetMoveDir(Constants::Direction d);
    void StopMove();
    void PlayerShoot();
    void TogglePause();
    void Quit();
    void Restart();

    bool Running() const;
    int GetScore() const;
    int GetLevel() const;
    int GetLives() const;
    int GetEnemiesKilled() const;
    GameState GetState() const;

    const Map& GetMap() const;
    const Tank* GetPlayer() const;
    const std::vector<Tank>& GetEnemies() const;
    const std::vector<Bullet>& GetBullets() const;
    const std::vector<Explosion>& GetExplosions() const;

private:
    void SpawnEnemy();
    void ProcessBullets();
    void ProcessEnemyAI();
    void ProcessPlayerMove();
    void CheckCollisions();
    void CheckBaseDestroyed();
    void NextLevel();
    void ResetPlayer();
    void CleanupDeadEnemies();
    int AliveEnemyCount() const;
    bool TankCollides(int x, int y, int ignoreIdx = -1, bool excludePlayer = false) const;
    bool TrySpawnBullet(Constants::Position spawn, Constants::Direction dir, bool fromPlayer);
    void AddExplosion(Constants::Position p);

    Map map;
    std::unique_ptr<Tank> player;
    std::vector<Tank> enemies;
    std::vector<Bullet> bullets;
    std::vector<Explosion> explosions;

    int score;
    int level;
    int lives;
    int enemySpawnTimer;
    int enemiesSpawned;
    int enemiesKilled;
    GameState state;

    // Smooth movement
    Constants::Direction moveDir = Constants::Direction::NONE;
    int moveCooldown = 0;
    static constexpr int MOVE_INTERVAL = 2;
};
