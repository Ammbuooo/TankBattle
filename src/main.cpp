#include "game.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <iostream>

using namespace ftxui;

// ═══════════════════════════════════════════════════════════════════
// Rendering helpers
// ═══════════════════════════════════════════════════════════════════

static std::string tankChar(Constants::Direction d, bool player) {
    switch (d) {
    case Constants::Direction::UP:    return player ? "▲█" : "△▀";
    case Constants::Direction::DOWN:  return player ? "▼█" : "▽▄";
    case Constants::Direction::LEFT:  return player ? "█◄" : "▌◁";
    case Constants::Direction::RIGHT: return player ? "█►" : "▐▷";
    default: return "??";
    }
}

static Color tankFg(bool player) {
    return player ? Color::GreenLight : Color::RedLight;
}

static Color tankBg(bool player) {
    return player ? Color::Green : Color::Red;
}

// Explosion character based on remaining timer
static std::string explosionChar(int timer) {
    if (timer >= 5) return "**";
    if (timer >= 3) return "* ";
    if (timer >= 1) return ". ";
    return "  ";
}

static Color explosionColor(int timer) {
    if (timer >= 5) return Color::Orange1;
    if (timer >= 3) return Color::Yellow;
    return Color::Grey50;
}

// ═══════════════════════════════════════════════════════════════════
// Main render function
// ═══════════════════════════════════════════════════════════════════

static Element RenderGame(const Game& game) {
    const auto& map = game.GetMap();
    const auto* player = game.GetPlayer();
    const auto& enemies = game.GetEnemies();
    const auto& bullets = game.GetBullets();
    const auto& explosions = game.GetExplosions();
    auto state = game.GetState();

    // ── Build explosion lookup ───────────────────────────────────
    // Map position -> highest timer explosion
    int explosionGrid[Constants::MAP_HEIGHT][Constants::MAP_WIDTH] = {};
    for (const auto& ex : explosions) {
        if (map.IsInBounds(ex.pos.x, ex.pos.y)) {
            if (ex.timer > explosionGrid[ex.pos.y][ex.pos.x])
                explosionGrid[ex.pos.y][ex.pos.x] = ex.timer;
        }
    }

    // ── Game map grid ────────────────────────────────────────────
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

            int exTimer = explosionGrid[y][x];
            auto ct = map.GetCell(x, y);

            // Render priority: explosion > bullet > tank > map
            if (exTimer > 0) {
                cell = explosionChar(exTimer);
                fg = explosionColor(exTimer);
                bg = Color::Black;
            } else if (hasBullet) {
                cell = "◉ ";
                fg = Color::YellowLight;
                bg = Color::Black;
            } else if (hasPlayer) {
                cell = tankChar(player->dir, true);
                fg = tankFg(true);
                bg = tankBg(true);
            } else if (hasEnemy) {
                cell = tankChar(enemyDir, false);
                fg = tankFg(false);
                bg = tankBg(false);
            } else if (ct == Constants::CellType::BRICK) {
                cell = "▓▓";
                fg = Color::Gold1;
                bg = Color::Orange3;
            } else if (ct == Constants::CellType::STEEL) {
                cell = "██";
                fg = Color::Grey70;
                bg = Color::Grey35;
            } else if (ct == Constants::CellType::BASE) {
                cell = player && player->alive ? "⬥ " : "✖ ";
                fg = player && player->alive ? Color::CyanLight : Color::Red;
                bg = Color::Black;
            } else {
                cell = ((x + y) % 2 == 0) ? "· " : "  ";
                fg = Color::Grey15;
                bg = Color::Black;
            }

            cols.push_back(text(cell) | color(fg) | bgcolor(bg));
        }
        rows.push_back(hbox(std::move(cols)));
    }

    auto gameArea = vbox(std::move(rows)) | borderDouble | color(Color::Grey50);

    // ── Title ────────────────────────────────────────────────────
    auto title = hbox({
        text(" ▰▰ ") | color(Color::GreenLight),
        text("TANK") | bold | color(Color::GreenLight),
        text(" BATTLE ") | bold | color(Color::Gold1),
        text("▰▰ ") | color(Color::RedLight),
    }) | center;

    // ── Status ───────────────────────────────────────────────────
    std::string statusText;
    Color statusColor;
    switch (state) {
    case GameState::PLAYING: statusText = "⚔  PLAYING  ⚔"; statusColor = Color::GreenLight; break;
    case GameState::PAUSED:  statusText = "⏸  PAUSED  ⏸";   statusColor = Color::YellowLight; break;
    case GameState::WIN:     statusText = "★  YOU WIN!  ★"; statusColor = Color::Gold1; break;
    case GameState::LOSE:    statusText = "☠  GAME OVER  ☠"; statusColor = Color::RedLight; break;
    }

    // ── HUD panels ───────────────────────────────────────────────
    std::string hearts;
    for (int i = 0; i < game.GetLives(); i++) hearts += "♥ ";
    if (hearts.empty()) hearts = "--";

    auto mkPanel = [](std::string label, Element content, Color c) {
        return vbox({
            text(" " + label + " ") | bold | color(c) | center | bgcolor(Color::Grey15),
            separator(),
            content | center,
        }) | border | color(c);
    };

    auto scorePanel  = mkPanel("SCORE",  text(std::to_string(game.GetScore())) | bold | color(Color::Yellow), Color::Yellow);
    auto levelPanel  = mkPanel("LEVEL",  text(std::to_string(game.GetLevel())) | bold | color(Color::Cyan), Color::Cyan);
    auto livesPanel  = mkPanel("LIVES",  text(hearts) | color(Color::RedLight), Color::RedLight);
    auto killsPanel  = mkPanel("KILLS",
        text(std::to_string(game.GetEnemiesKilled()) + "/" + std::to_string(Constants::ENEMIES_PER_LEVEL)) | color(Color::Orange1),
        Color::Orange1);
    auto statusPanel = mkPanel("STATUS", text(statusText) | bold | color(statusColor), statusColor);

    auto hudTop = hbox({ scorePanel | flex, levelPanel | flex, livesPanel | flex, killsPanel | flex, statusPanel | flex });

    // ── Controls bar ──────────────────────────────────────────────
    auto keyHint = [](std::string key, std::string action, Color c) {
        return hbox({
            text(" ") | size(WIDTH, EQUAL, 0),
            text(key) | bold | color(c),
            text(" " + action) | color(Color::Grey70),
        });
    };

    auto controls = hbox({
        keyHint("▲▼◄►", "Move",  Color::Cyan),
        text(" │ ") | color(Color::Grey30),
        keyHint("Space", "Shoot", Color::Yellow),
        text(" │ ") | color(Color::Grey30),
        keyHint("WASD", "Move",  Color::Cyan),
        text(" │ ") | color(Color::Grey30),
        keyHint("P", "Pause",   Color::Magenta),
        text(" │ ") | color(Color::Grey30),
        keyHint("R", "Restart", Color::Green),
        text(" │ ") | color(Color::Grey30),
        keyHint("Q", "Quit",    Color::Red),
    }) | center | border | color(Color::Grey35);

    // ── Layout ────────────────────────────────────────────────────
    return vbox({
        title,
        separator(),
        gameArea,
        separator(),
        hudTop,
        separator(),
        controls,
    });
}

// ═══════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════

int main() {
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

    auto renderer = Renderer([&] { return RenderGame(game); });

    auto component = renderer | CatchEvent([&](Event event) {
        if (event == Event::Custom) return true;

        if (event == Event::Character('q') || event == Event::Character('Q'))
            game.Quit();
        if (event == Event::Character('r') || event == Event::Character('R'))
            game.Restart();
        if (event == Event::Character('p') || event == Event::Character('P'))
            game.TogglePause();
        if (event == Event::ArrowUp)    game.SetMoveDir(Constants::Direction::UP);
        if (event == Event::ArrowDown)  game.SetMoveDir(Constants::Direction::DOWN);
        if (event == Event::ArrowLeft)  game.SetMoveDir(Constants::Direction::LEFT);
        if (event == Event::ArrowRight) game.SetMoveDir(Constants::Direction::RIGHT);
        if (event == Event::Character('w') || event == Event::Character('W'))
            game.SetMoveDir(Constants::Direction::UP);
        if (event == Event::Character('s') || event == Event::Character('S'))
            game.SetMoveDir(Constants::Direction::DOWN);
        if (event == Event::Character('a') || event == Event::Character('A'))
            game.SetMoveDir(Constants::Direction::LEFT);
        if (event == Event::Character('d') || event == Event::Character('D'))
            game.SetMoveDir(Constants::Direction::RIGHT);
        if (event == Event::Character(' ')) game.PlayerShoot();

        if (!game.Running()) screen.Exit();
        return true;
    });

    screen.Loop(component);

    updateRunning = false;
    if (updateThread.joinable()) updateThread.join();

    std::cout << "\n";
    std::cout << "  ╔══════════════════════╗\n";
    std::cout << "  ║     GAME OVER        ║\n";
    std::cout << "  ╠══════════════════════╣\n";
    std::cout << "  ║  Score: " << std::setw(12) << game.GetScore() << " ║\n";
    std::cout << "  ║  Level: " << std::setw(12) << game.GetLevel() << " ║\n";
    std::cout << "  ╚══════════════════════╝\n\n";

    return 0;
}
