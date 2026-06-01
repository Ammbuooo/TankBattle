# 坦克大战 (Tank Battle)

使用 C++17 和 [FTXUI](https://github.com/ArthurSonzogni/FTXUI) 终端 UI 库实现的经典坦克大战游戏。

## 游戏特色

- 经典的俯视角坦克大战玩法
- 3 个独特的关卡布局，难度逐步提升
- 砖墙（可被摧毁）与钢墙（不可摧毁）
- 带有随机移动和射击行为的敌方 AI
- 分数追踪与多条生命
- 渐进式关卡（每关 20 个敌人）
- 基于终端的彩色渲染界面

## 操作方式

| 按键 | 动作 |
|---|---|
| 方向键 | 移动坦克 |
| 空格键 | 开火 |
| P | 暂停 / 继续 |
| Q | 退出游戏 |

## 环境要求

- 支持 C++17 的编译器（GCC 8+、Clang 7+、MSVC 2019+）
- CMake 3.14 或更高版本
- 网络连接（首次构建时会自动下载 FTXUI）

## 构建与运行

```bash
# 克隆仓库
git clone https://github.com/YOUR_USERNAME/tank-battle.git
cd tank-battle

# 配置并构建
cmake -B build
cmake --build build

# 运行游戏
./build/tank_battle
```
