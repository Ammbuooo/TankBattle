# Tank Battle (坦克大战)

A classic tank battle game implemented in C++17 with the [FTXUI](https://github.com/ArthurSonzogni/FTXUI) terminal UI library.

## Features

- Classic top-down tank battle gameplay
- 3 unique level layouts with increasing difficulty
- Brick walls (destructible) and steel walls (indestructible)
- Enemy AI with randomized movement and shooting
- Score tracking and multiple lives
- Progressive levels (20 enemies per level)
- Terminal-based UI with color rendering

## Controls

| Key | Action |
|---|---|
| Arrow Keys | Move tank |
| Space | Shoot |
| P | Pause / Resume |
| Q | Quit game |

## Requirements

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14 or higher
- Internet connection (first build downloads FTXUI automatically)

## Build & Run

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/tank-battle.git
cd tank-battle

# Configure and build
cmake -B build
cmake --build build

# Run the game
./build/tank_battle
```

### Windows (Visual Studio)

```powershell
cmake -B build
cmake --build build --config Release
.\build\Release\tank_battle.exe
```

## Project Structure

```
tank-battle/
├── CMakeLists.txt          # Build configuration
├── README.md
├── include/
│   ├── constants.hpp       # Game constants and shared types
│   ├── tank.hpp            # Tank entity
│   ├── bullet.hpp          # Bullet entity
│   ├── map.hpp             # Game map & level data
│   └── game.hpp            # Core game logic
└── src/
    ├── main.cpp            # Entry point & FTXUI integration
    ├── tank.cpp
    ├── bullet.cpp
    ├── map.cpp
    └── game.cpp
```

## Contributing

Contributions are welcome. Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to the branch (`git push origin feature/your-feature`)
5. Open a Pull Request

### Ideas for Contributions

- Sound effects
- Power-ups (speed boost, multi-shot, shield)
- More level designs
- Two-player co-op mode
- High score persistence

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) — C++ Functional Terminal User Interface library
- Inspired by the classic NES *Battle City* (坦克大战)
