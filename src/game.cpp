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

void Game::TogglePause() {
    if (state == GameState::PLAYING) state = GameState::PAUSED;
    else if (state == GameState::PAUSED) state = GameState::PLAYING;
}

void Game::Quit() {
    state = GameState::LOSE;
}

void Game::ResetPlayer() {
    player = std::make_unique<Tank>(12, 22, Constants::Direction::UP, Constants::TankType::PLAYER);
    player->hp = 1;
}

bool Game::TankCollides(int x, int y, int ignoreIdx) const {
    if (!map.IsInBounds(x, y)) return true;
    if (map.IsSolid(x, y)) return true;
    if (map.GetCell(x, y) == Constants::CellType::BASE) return true;

    if (player && player->alive && player->pos.x == x && player->pos.y == y)
        return true;

    for (size_t i = 0; i < enemies.size(); i++) {
        if ((int)i == ignoreIdx) continue;
        if (enemies[i].alive && enemies[i].pos.x == x && enemies[i].pos.y == y)
            return true;
    }
    return false;
}

void Game::PlayerMove(Constants::Direction d) {
    if (!player || !player->alive) return;
    if (state != GameState::PLAYING) return;

    player->dir = d;
    int prevX = player->pos.x;
    int prevY = player->pos.y;

    player->Move();

    if (TankCollides(player->pos.x, player->pos.y)) {
        player->pos.x = prevX;
        player->pos.y = prevY;
    }
}

void Game::PlayerShoot() {
    if (!player || !player->alive || !player->CanShoot()) return;
    if (state != GameState::PLAYING) return;

    player->OnShoot();
    bullets.emplace_back();
    bullets.back().Fire(player->BulletSpawn(), player->dir, true);
}

void Game::SpawnEnemy() {
    if ((int)enemies.size() >= Constants::MAX_ENEMIES_ON_SCREEN) return;
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

    enemySpawnTimer++;
    if (enemySpawnTimer >= Constants::ENEMY_SPAWN_INTERVAL) {
        SpawnEnemy();
    }

    ProcessEnemyAI();
    ProcessBullets();
    CheckCollisions();
    CheckBaseDestroyed();

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
            bullets.emplace_back();
            bullets.back().Fire(enemy.BulletSpawn(), enemy.dir, false);
        }
    }
}

void Game::ProcessBullets() {
    for (auto& bullet : bullets) {
        if (!bullet.active) continue;

        bullet.Move();

        int bx = bullet.pos.x;
        int by = bullet.pos.y;

        if (!map.IsInBounds(bx, by)) {
            bullet.Deactivate();
            continue;
        }

        auto cell = map.GetCell(bx, by);

        if (cell == Constants::CellType::STEEL) {
            bullet.Deactivate();
            continue;
        }

        if (cell == Constants::CellType::BRICK) {
            map.SetCell(bx, by, Constants::CellType::EMPTY);
            bullet.Deactivate();
            continue;
        }

        if (cell == Constants::CellType::BASE) {
            map.SetCell(bx, by, Constants::CellType::EMPTY);
            bullet.Deactivate();
            continue;
        }

        if (!bullet.fromPlayer && player && player->alive &&
            player->pos.x == bx && player->pos.y == by) {
            player->alive = false;
            bullet.Deactivate();
            lives--;
            if (lives > 0) {
                ResetPlayer();
            } else {
                state = GameState::LOSE;
            }
            continue;
        }

        if (bullet.fromPlayer) {
            for (auto& enemy : enemies) {
                if (enemy.alive && enemy.pos.x == bx && enemy.pos.y == by) {
                    enemy.alive = false;
                    bullet.Deactivate();
                    score += 100;
                    enemiesKilled++;
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
            enemy.alive = false;
            lives--;
            if (lives > 0) {
                ResetPlayer();
            } else {
                state = GameState::LOSE;
            }
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
    enemiesSpawned = 0;
    enemiesKilled = 0;
    enemySpawnTimer = 0;
    map.LoadLevel(level);
    ResetPlayer();
    score += 500;
}
