#include "game.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();
    Game game;

    std::atomic<bool> updateRunning{true};

    // Background thread for game state updates
    std::thread updateThread([&] {
        while (updateRunning) {
            game.Update();
            // Post a custom event to trigger re-render
            screen.PostEvent(Event::Custom);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(Constants::TICK_INTERVAL_MS));
        }
    });

    auto renderer = Renderer([&] {
        return game.Render();
    });

    auto component = renderer | CatchEvent([&](Event event) {
        // Custom event from update thread — just re-render
        if (event == Event::Custom) {
            return true;
        }

        game.HandleInput(event);

        if (!game.Running()) {
            screen.Exit();
        }

        return true;
    });

    screen.Loop(component);

    // Stop background thread
    updateRunning = false;
    if (updateThread.joinable()) {
        updateThread.join();
    }

    // Show final results
    std::cout << "\n=== GAME OVER ===\n";
    std::cout << "Final Score: " << game.Score() << "\n";
    std::cout << "Level Reached: " << game.Level() << "\n\n";

    return 0;
}
