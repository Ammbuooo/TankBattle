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

// ── Rendering helpers ──────────────────────────────────────────────

static Color cellFg(Constants::CellType t) {
    switch (t) {
    case Constants::CellType::BRICK: return Color::Gold1;
    case Constants::CellType::STEEL: return Color::Grey70;
    case Constants::CellType::BASE:  return Color::Cyan;
    default: return Color::White;
    }
}

static Color cellBg(Constants::CellType t) {
    switch (t) {
    case Constants::CellType::BRICK: return Color::Orange3;
    case Constants::CellType::STEEL: return Color::Grey35;
    case Constants::CellType::BASE:  return Color::Black;
    default: return Color::Black;
    }
}

static std::string tankChar(Constants::Direction d, bool player) {
    // Player uses filled arrow + block, enemy uses outlined
    switch (d) {
    case Constants::Direction::UP:    return player ? "▲█" : "△█";
    case Constants::Direction::DOWN:  return player ? "▼█" : "▽█";
    case Constants::Direction::LEFT:  return player ? "█◄" : "█◁";
    case Constants::Direction::RIGHT: return player ? "█►" : "█▷";
    }
    return "??";
}

static Color tankFg(bool player) {
    return player ? Color::GreenLight : Color::RedLight;
}

static Color tankBg(bool player) {
    return player ? Color::Green : Color::Red;
}

// ── Main render function ───────────────────────────────────────────

static Element RenderGame(const Game& game) {
    const auto& map = game.GetMap();
    const auto* player = game.GetPlayer();
    const auto& enemies = game.GetEnemies();
    const auto& bullets = game.GetBullets();
    auto state = game.GetState();

    // ── Game map grid ─────────────────────────────────────────
    Elements rows;
    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        Elements cols;
        for (int x = 0; x < Constants::MAP_WIDTH; x++) {
            std::string cell = "  ";
            Color fg = Color::White;
            Color bg = Color::Black;

            // --- Entities at this cell ---
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

            auto ct = map.GetCell(x, y);

            // --- Render priority: bullet > tank > map ---
            if (hasBullet) {
                cell = "● ";
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
                fg = cellFg(ct);
                bg = cellBg(ct);
            } else if (ct == Constants::CellType::STEEL) {
                cell = "██";
                fg = cellFg(ct);
                bg = cellBg(ct);
            } else if (ct == Constants::CellType::BASE) {
                cell = "⬥ ";
                fg = Color::CyanLight;
                bg = Color::Black;
            } else {
                // empty ground — subtle grid dot
                cell = ((x + y) % 2 == 0) ? "· " : "  ";
                fg = Color::Grey19;
                bg = Color::Black;
            }

            cols.push_back(text(cell) | color(fg) | bgcolor(bg));
        }
        rows.push_back(hbox(std::move(cols)));
    }

    auto gameArea = vbox(std::move(rows)) | borderDouble;

    // ── HUD ───────────────────────────────────────────────────
    auto title = text(" TANK BATTLE ") | bold | color(Color::Gold1) | center;

    std::string statusText;
    Color statusColor = Color::White;
    switch (state) {
    case GameState::PLAYING: statusText = "⚔  PLAYING  ⚔"; statusColor = Color::GreenLight; break;
    case GameState::PAUSED:  statusText = "⏸  PAUSED  ⏸";   statusColor = Color::Yellow;   break;
    case GameState::WIN:     statusText = "★  YOU WIN!  ★"; statusColor = Color::Gold1;    break;
    case GameState::LOSE:    statusText = "☠  GAME OVER  ☠"; statusColor = Color::Red;      break;
    }

    // Lives as hearts
    std::string hearts;
    for (int i = 0; i < game.GetLives(); i++) hearts += "♥ ";
    if (hearts.empty()) hearts = "--";

    auto scoreBox = window(text(" SCORE "), text(std::to_string(game.GetScore())) | center | bold | color(Color::Yellow));
    auto levelBox = window(text(" LEVEL "), text(std::to_string(game.GetLevel())) | center | bold | color(Color::Cyan));
    auto livesBox = window(text(" LIVES "), text(hearts) | center | color(Color::RedLight));
    auto killsBox = window(text(" KILLS "),
        text(std::to_string(game.GetEnemiesKilled()) + "/" + std::to_string(Constants::ENEMIES_PER_LEVEL))
        | center | color(Color::Orange1));
    auto statusBox = window(text(" STATUS "), text(statusText) | center | bold | color(statusColor));

    auto hudRow1 = hbox({ scoreBox | flex, levelBox | flex, livesBox | flex });
    auto hudRow2 = hbox({ killsBox | flex, statusBox | flex });

    // ── Controls help ──────────────────────────────────────────
    auto controls = hbox({
        text(" ▲▼◄► Move ") | color(Color::Grey70),
        text(" │ "),
        text(" Space Shoot ") | color(Color::Grey70),
        text(" │ "),
        text(" P Pause ") | color(Color::Grey70),
        text(" │ "),
        text(" Q Quit ") | color(Color::Grey70),
    }) | center;

    // ── Final layout ───────────────────────────────────────────
    return vbox({
        title,
        separator(),
        gameArea,
        separator(),
        hudRow1,
        hudRow2,
        separator(),
        controls,
    });
}

// ── Main ────────────────────────────────────────────────────────────

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
        if (event == Event::Character('p') || event == Event::Character('P'))
            game.TogglePause();
        if (event == Event::ArrowUp)    game.PlayerMove(Constants::Direction::UP);
        if (event == Event::ArrowDown)  game.PlayerMove(Constants::Direction::DOWN);
        if (event == Event::ArrowLeft)  game.PlayerMove(Constants::Direction::LEFT);
        if (event == Event::ArrowRight) game.PlayerMove(Constants::Direction::RIGHT);
        // WASD fallback
        if (event == Event::Character('w') || event == Event::Character('W'))
            game.PlayerMove(Constants::Direction::UP);
        if (event == Event::Character('s') || event == Event::Character('S'))
            game.PlayerMove(Constants::Direction::DOWN);
        if (event == Event::Character('a') || event == Event::Character('A'))
            game.PlayerMove(Constants::Direction::LEFT);
        if (event == Event::Character('d') || event == Event::Character('D'))
            game.PlayerMove(Constants::Direction::RIGHT);
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
