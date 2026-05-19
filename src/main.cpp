#include "game.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

static ftxui::Element RenderGame(const Game& game) {
    using namespace ftxui;

    const auto& map = game.GetMap();
    const auto* player = game.GetPlayer();
    const auto& enemies = game.GetEnemies();
    const auto& bullets = game.GetBullets();
    auto state = game.GetState();

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
        text(" Score: " + std::to_string(game.GetScore()) + " ") | border,
        text(" Level: " + std::to_string(game.GetLevel()) + " ") | border,
        text(" Lives: " + std::to_string(game.GetLives()) + " ") | border,
        text(" " + statusText + " ") | border | color(Color::Yellow),
        text(" Kills: " + std::to_string(game.GetEnemiesKilled()) + "/" +
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

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();
    Game game;

    std::atomic<bool> updateRunning{true};

    std::thread updateThread([&] {
        while (updateRunning) {
            game.Update();
            screen.PostEvent(Event::Custom);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Constants::TICK_INTERVAL_MS));
        }
    });

    auto renderer = Renderer([&] {
        return RenderGame(game);
    });

    auto component = renderer | CatchEvent([&](Event event) {
        if (event == Event::Custom) {
            return true;
        }

        if (event == Event::Character('q') || event == Event::Character('Q')) {
            game.Quit();
        }
        if (event == Event::Character('p') || event == Event::Character('P')) {
            game.TogglePause();
        }
        if (event == Event::ArrowUp)    { game.PlayerMove(Constants::Direction::UP); }
        if (event == Event::ArrowDown)  { game.PlayerMove(Constants::Direction::DOWN); }
        if (event == Event::ArrowLeft)  { game.PlayerMove(Constants::Direction::LEFT); }
        if (event == Event::ArrowRight) { game.PlayerMove(Constants::Direction::RIGHT); }
        if (event == Event::Character(' ')) { game.PlayerShoot(); }

        if (!game.Running()) {
            screen.Exit();
        }

        return true;
    });

    screen.Loop(component);

    updateRunning = false;
    if (updateThread.joinable()) {
        updateThread.join();
    }

    std::cout << "\n=== GAME OVER ===\n";
    std::cout << "Final Score: " << game.GetScore() << "\n";
    std::cout << "Level Reached: " << game.GetLevel() << "\n\n";

    return 0;
}
