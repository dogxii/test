# 机器人 3D 漫游世界

C++17 / OpenGL 图形学实训项目。

玩家可以在 Dust2 风格 OBJ 地图中漫游、切换视角并射击敌人。项目使用固定管线，保留圆环、小球和方块机器人等任务书要求。

## 功能

- 2.5D、第一人称、观察者三种视角
- OBJ 地图、斜坡高度与 2.5D 遮挡处理
- 可见子弹、敌人倒地与目标进度
- 启动页、HUD 和暂停页

## 运行

需要 CMake、GLFW、GLM 和 OpenGL。

macOS / Linux：

```bash
sh dev.sh
```

Windows：

```bat
dev.bat
```

Windows 缺少 vcpkg 时：

```bat
dev.bat setup-vcpkg
```

只编译或清理：

```bash
sh dev.sh build
sh dev.sh clean
```

## 操作

| 按键 | 功能 |
| --- | --- |
| `WASD` | 移动或转向 |
| `Space` | 射击；观察者上升 |
| `Shift` | 观察者下降 |
| `C` | 切换视角 |
| `Esc` | 暂停 |
| `Enter` | 开始或继续 |
| `R` | 重新开始 |
| `Q` | 退出 |

Windows exe 已内置 Dust2 地图，也可以用 `de_dust2-cs-map` 中的 OBJ 覆盖调试。
