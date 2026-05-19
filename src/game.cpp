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

int Game::Score() const { return score; }
int Game::Level() const { return level; }
int Game::Lives() const { return lives; }
GameState Game::GetState() const { return state; }

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

void Game::MovePlayer(Constants::Direction d) {
    if (!player || !player->alive) return;

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

void Game::HandleInput(const ftxui::Event& event) {
    if (event == ftxui::Event::Character('q') || event == ftxui::Event::Character('Q')) {
        state = GameState::LOSE;
        return;
    }

    if (event == ftxui::Event::Character('p') || event == ftxui::Event::Character('P')) {
        if (state == GameState::PLAYING) state = GameState::PAUSED;
        else if (state == GameState::PAUSED) state = GameState::PLAYING;
        return;
    }

    if (state != GameState::PLAYING) return;

    if (event == ftxui::Event::ArrowUp) {
        MovePlayer(Constants::Direction::UP);
        return;
    }
    if (event == ftxui::Event::ArrowDown) {
        MovePlayer(Constants::Direction::DOWN);
        return;
    }
    if (event == ftxui::Event::ArrowLeft) {
        MovePlayer(Constants::Direction::LEFT);
        return;
    }
    if (event == ftxui::Event::ArrowRight) {
        MovePlayer(Constants::Direction::RIGHT);
        return;
    }
    if (event == ftxui::Event::Character(' ')) {
        PlayerShoot();
        return;
    }
}

void Game::Update() {
    if (state != GameState::PLAYING) return;

    // Update player cooldowns
    if (player && player->alive) {
        player->Update();
    }

    // Spawn enemies
    enemySpawnTimer++;
    if (enemySpawnTimer >= Constants::ENEMY_SPAWN_INTERVAL) {
        SpawnEnemy();
    }

    // Enemy AI
    ProcessEnemyAI();

    // Bullets
    ProcessBullets();

    // Collisions
    CheckCollisions();

    // Base check
    CheckBaseDestroyed();

    // Win condition
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

ftxui::Element Game::Render() const {
    using namespace ftxui;

    Elements rows;

    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        Elements cols;
        for (int x = 0; x < Constants::MAP_WIDTH; x++) {
            std::string cell = "  ";
            Color fg = Color::White;
            Color bg = Color::Black;

            bool hasPlayer = player && player->alive && player->pos.x == x && player->pos.y == y;
            bool hasEnemy = false;
            Constants::Direction enemyDir = Constants::Direction::UP;
            for (const auto& e : enemies) {
                if (e.alive && e.pos.x == x && e.pos.y == y) {
                    hasEnemy = true;
                    enemyDir = e.dir;
                    break;
                }
            }
            bool hasBullet = false;
            for (const auto& b : bullets) {
                if (b.active && b.pos.x == x && b.pos.y == y) {
                    hasBullet = true;
                    break;
                }
            }

            auto cellType = map.GetCell(x, y);

            if (hasPlayer) {
                switch (player->dir) {
                case Constants::Direction::UP:    cell = "▲ "; break;
                case Constants::Direction::DOWN:  cell = "▼ "; break;
                case Constants::Direction::LEFT:  cell = "◄ "; break;
                case Constants::Direction::RIGHT: cell = "► "; break;
                }
                fg = Color::Green;
            } else if (hasEnemy) {
                switch (enemyDir) {
                case Constants::Direction::UP:    cell = "▲ "; break;
                case Constants::Direction::DOWN:  cell = "▼ "; break;
                case Constants::Direction::LEFT:  cell = "◄ "; break;
                case Constants::Direction::RIGHT: cell = "► "; break;
                }
                fg = Color::Red;
            } else if (hasBullet) {
                cell = "● ";
                fg = Color::Yellow;
            } else if (cellType == Constants::CellType::BRICK) {
                cell = "▓▓";
                fg = Color::Orange1;
                bg = Color::Orange1;
            } else if (cellType == Constants::CellType::STEEL) {
                cell = "██";
                fg = Color::Grey50;
                bg = Color::Grey50;
            } else if (cellType == Constants::CellType::BASE) {
                cell = "⬥ ";
                fg = Color::Cyan;
            }

            cols.push_back(text(cell) | color(fg) | bgcolor(bg));
        }
        rows.push_back(hbox(std::move(cols)));
    }

    auto gameArea = vbox(std::move(rows)) | border;

    std::string statusText;
    switch (state) {
    case GameState::PLAYING: statusText = "PLAYING"; break;
    case GameState::PAUSED:  statusText = "PAUSED";  break;
    case GameState::WIN:     statusText = "YOU WIN!"; break;
    case GameState::LOSE:    statusText = "GAME OVER"; break;
    }

    auto hud = hbox({
        text(" Score: " + std::to_string(score) + " ") | border,
        text(" Level: " + std::to_string(level) + " ") | border,
        text(" Lives: " + std::to_string(lives) + " ") | border,
        text(" " + statusText + " ") | border | color(Color::Yellow),
        text(" Kills: " + std::to_string(enemiesKilled) + "/" +
             std::to_string(Constants::ENEMIES_PER_LEVEL) + " ") | border,
    });

    auto controls = text(" Arrow Keys: Move  |  Space: Shoot  |  P: Pause  |  Q: Quit");

    return vbox({
        gameArea,
        separator(),
        hud | center,
        separator(),
        controls | center | dim
    });
}
