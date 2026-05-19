#include "../include/game.hpp"
#include <cstdlib>
#include <algorithm>

Game::Game()
    : score(0), level(1), lives(Constants::PLAYER_LIVES),
      enemySpawnTimer(0), enemiesSpawned(0), enemiesKilled(0),
      state(GameState::PLAYING)
{
    map.LoadLevel(level);
    ResetPlayer();
}

bool Game::Running() const {
    return state == GameState::PLAYING || state == GameState::PAUSED;
}

int Game::GetScore() const { return score; }
int Game::GetLevel() const { return level; }
int Game::GetLives() const { return lives; }
int Game::GetEnemiesKilled() const { return enemiesKilled; }
GameState Game::GetState() const { return state; }
const Map& Game::GetMap() const { return map; }
const Tank* Game::GetPlayer() const { return player.get(); }
const std::vector<Tank>& Game::GetEnemies() const { return enemies; }
const std::vector<Bullet>& Game::GetBullets() const { return bullets; }
const std::vector<Explosion>& Game::GetExplosions() const { return explosions; }

void Game::TogglePause() {
    if (state == GameState::PLAYING) state = GameState::PAUSED;
    else if (state == GameState::PAUSED) state = GameState::PLAYING;
}

void Game::Quit() {
    state = GameState::LOSE;
}

void Game::Restart() {
    score = 0;
    level = 1;
    lives = Constants::PLAYER_LIVES;
    enemiesSpawned = 0;
    enemiesKilled = 0;
    enemySpawnTimer = 0;
    enemies.clear();
    bullets.clear();
    explosions.clear();
    state = GameState::PLAYING;
    map.LoadLevel(level);
    ResetPlayer();
}

void Game::ResetPlayer() {
    player = std::make_unique<Tank>(12, 22, Constants::Direction::UP, Constants::TankType::PLAYER);
    player->hp = 1;
    moveDir = Constants::Direction::NONE;
    moveCooldown = 0;
}

int Game::AliveEnemyCount() const {
    int count = 0;
    for (const auto& e : enemies)
        if (e.alive) count++;
    return count;
}

void Game::CleanupDeadEnemies() {
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const Tank& t) { return !t.alive; }),
        enemies.end());
}

bool Game::TankCollides(int x, int y, int ignoreIdx, bool excludePlayer) const {
    if (!map.IsInBounds(x, y)) return true;
    if (map.IsSolid(x, y)) return true;
    if (map.GetCell(x, y) == Constants::CellType::BASE) return true;

    if (!excludePlayer && player && player->alive && player->pos.x == x && player->pos.y == y)
        return true;

    for (size_t i = 0; i < enemies.size(); i++) {
        if ((int)i == ignoreIdx) continue;
        if (enemies[i].alive && enemies[i].pos.x == x && enemies[i].pos.y == y)
            return true;
    }
    return false;
}

void Game::AddExplosion(Constants::Position p) {
    explosions.push_back({p, 6});
}

void Game::SetMoveDir(Constants::Direction d) {
    if (!player || !player->alive) return;
    if (state != GameState::PLAYING) return;

    player->dir = d;
    moveDir = d;
    moveCooldown = 0;
}

void Game::StopMove() {
    moveDir = Constants::Direction::NONE;
}

void Game::ProcessPlayerMove() {
    if (moveDir == Constants::Direction::NONE) return;

    if (moveCooldown > 0) {
        moveCooldown--;
        return;
    }

    if (!player || !player->alive) return;

    player->dir = moveDir;
    int prevX = player->pos.x;
    int prevY = player->pos.y;

    switch (moveDir) {
    case Constants::Direction::UP:    player->pos.y--; break;
    case Constants::Direction::DOWN:  player->pos.y++; break;
    case Constants::Direction::LEFT:  player->pos.x--; break;
    case Constants::Direction::RIGHT: player->pos.x++; break;
    default: return;
    }

    if (TankCollides(player->pos.x, player->pos.y, -1, true)) {
        player->pos.x = prevX;
        player->pos.y = prevY;
    }

    moveCooldown = MOVE_INTERVAL;
}

// Shoot helper: checks spawn position for immediate wall hit
bool Game::TrySpawnBullet(Constants::Position spawn, Constants::Direction dir, bool fromPlayer) {
    if (!map.IsInBounds(spawn.x, spawn.y)) return false;

    auto cell = map.GetCell(spawn.x, spawn.y);

    if (cell == Constants::CellType::STEEL) return false;
    if (cell == Constants::CellType::BRICK) {
        map.SetCell(spawn.x, spawn.y, Constants::CellType::EMPTY);
        AddExplosion(spawn);
        return false; // bullet consumed by wall
    }
    if (cell == Constants::CellType::BASE) {
        map.SetCell(spawn.x, spawn.y, Constants::CellType::EMPTY);
        AddExplosion(spawn);
        return false;
    }

    // Check tanks at spawn
    if (fromPlayer) {
        for (auto& e : enemies) {
            if (e.alive && e.pos.x == spawn.x && e.pos.y == spawn.y) {
                e.alive = false;
                AddExplosion(spawn);
                score += 100;
                enemiesKilled++;
                return false;
            }
        }
    } else {
        if (player && player->alive && player->pos.x == spawn.x && player->pos.y == spawn.y) {
            player->alive = false;
            AddExplosion(spawn);
            lives--;
            if (lives <= 0) state = GameState::LOSE;
            else ResetPlayer();
            return false;
        }
    }

    bullets.emplace_back();
    bullets.back().Fire(spawn, dir, fromPlayer);
    return true;
}

void Game::PlayerShoot() {
    if (!player || !player->alive || !player->CanShoot()) return;
    if (state != GameState::PLAYING) return;

    player->OnShoot();
    TrySpawnBullet(player->BulletSpawn(), player->dir, true);
}

void Game::SpawnEnemy() {
    if (AliveEnemyCount() >= Constants::MAX_ENEMIES_ON_SCREEN) return;
    if (enemiesSpawned >= Constants::ENEMIES_PER_LEVEL) return;

    auto spawns = map.EnemySpawns();
    int idx = rand() % spawns.size();
    auto sp = spawns[idx];

    if (TankCollides(sp.x, sp.y)) return;

    enemies.emplace_back(sp.x, sp.y, Constants::Direction::DOWN, Constants::TankType::ENEMY);
    enemiesSpawned++;
    enemySpawnTimer = 0;
}

void Game::Update() {
    if (state != GameState::PLAYING) return;

    if (player && player->alive) {
        player->Update();
    }

    ProcessPlayerMove();

    enemySpawnTimer++;
    if (enemySpawnTimer >= Constants::ENEMY_SPAWN_INTERVAL) {
        SpawnEnemy();
    }

    ProcessEnemyAI();
    ProcessBullets();
    CheckCollisions();
    CheckBaseDestroyed();

    // Decay explosions
    for (auto& ex : explosions) ex.timer--;
    explosions.erase(
        std::remove_if(explosions.begin(), explosions.end(),
                       [](const Explosion& e) { return e.timer <= 0; }),
        explosions.end());

    CleanupDeadEnemies();

    if (enemiesKilled >= Constants::ENEMIES_PER_LEVEL && state == GameState::PLAYING) {
        NextLevel();
    }
}

void Game::ProcessEnemyAI() {
    for (size_t i = 0; i < enemies.size(); i++) {
        auto& enemy = enemies[i];
        if (!enemy.alive) continue;

        enemy.Update();

        if (rand() % 30 == 0) {
            int d = rand() % 4;
            switch (d) {
            case 0: enemy.dir = Constants::Direction::UP;    break;
            case 1: enemy.dir = Constants::Direction::DOWN;  break;
            case 2: enemy.dir = Constants::Direction::LEFT;  break;
            case 3: enemy.dir = Constants::Direction::RIGHT; break;
            }
        }

        int prevX = enemy.pos.x;
        int prevY = enemy.pos.y;
        enemy.Move();

        if (TankCollides(enemy.pos.x, enemy.pos.y, (int)i)) {
            enemy.pos.x = prevX;
            enemy.pos.y = prevY;
            int d = rand() % 4;
            switch (d) {
            case 0: enemy.dir = Constants::Direction::UP;    break;
            case 1: enemy.dir = Constants::Direction::DOWN;  break;
            case 2: enemy.dir = Constants::Direction::LEFT;  break;
            case 3: enemy.dir = Constants::Direction::RIGHT; break;
            }
        }

        if (enemy.CanShoot() && rand() % Constants::ENEMY_SHOOT_COOLDOWN == 0) {
            enemy.OnShoot();
            TrySpawnBullet(enemy.BulletSpawn(), enemy.dir, false);
        }
    }
}

void Game::ProcessBullets() {
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;

        for (int step = 0; step < Constants::BULLET_SPEED; step++) {
            bullet.Move();

            int bx = bullet.pos.x;
            int by = bullet.pos.y;

            if (!map.IsInBounds(bx, by)) {
                bullet.Deactivate();
                break;
            }

            auto cell = map.GetCell(bx, by);

            if (cell == Constants::CellType::STEEL) {
                AddExplosion({bx, by});
                bullet.Deactivate();
                break;
            }

            if (cell == Constants::CellType::BRICK) {
                map.SetCell(bx, by, Constants::CellType::EMPTY);
                AddExplosion({bx, by});
                bullet.Deactivate();
                break;
            }

            if (cell == Constants::CellType::BASE) {
                map.SetCell(bx, by, Constants::CellType::EMPTY);
                AddExplosion({bx, by});
                bullet.Deactivate();
                break;
            }

            if (!bullet.fromPlayer && player && player->alive &&
                player->pos.x == bx && player->pos.y == by) {
                player->alive = false;
                AddExplosion({bx, by});
                bullet.Deactivate();
                lives--;
                if (lives <= 0) state = GameState::LOSE;
                else ResetPlayer();
                break;
            }

            if (bullet.fromPlayer) {
                bool hit = false;
                for (auto& enemy : enemies) {
                    if (enemy.alive && enemy.pos.x == bx && enemy.pos.y == by) {
                        enemy.alive = false;
                        AddExplosion({bx, by});
                        score += 100;
                        enemiesKilled++;
                        hit = true;
                        break;
                    }
                }
                if (hit) {
                    bullet.Deactivate();
                    break;
                }
            }
        }
    }

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
                       [](const Bullet& b) { return !b.active; }),
        bullets.end());
}

void Game::CheckCollisions() {
    if (!player || !player->alive) return;

    for (auto& enemy : enemies) {
        if (enemy.alive && player->pos == enemy.pos) {
            player->alive = false;
            AddExplosion(player->pos);
            enemy.alive = false;
            AddExplosion(enemy.pos);
            lives--;
            if (lives <= 0) state = GameState::LOSE;
            else ResetPlayer();
            break;
        }
    }
}

void Game::CheckBaseDestroyed() {
    auto bp = map.BasePosition();
    if (map.GetCell(bp.x, bp.y) != Constants::CellType::BASE) {
        state = GameState::LOSE;
    }
}

void Game::NextLevel() {
    level++;
    enemies.clear();
    bullets.clear();
    explosions.clear();
    enemiesSpawned = 0;
    enemiesKilled = 0;
    enemySpawnTimer = 0;
    map.LoadLevel(level);
    ResetPlayer();
    score += 500;
}
