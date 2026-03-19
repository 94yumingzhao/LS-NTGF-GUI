# LS-NTGF-GUI: Qt6 Visualization & Control Interface

> **NEVER delete any files or folders without explicit user instruction**

Qt6 GUI 前端, 通过 QProcess 启动 ls-ntgf-all 子进程求解.

## Build & Run

```bash
cmake --preset release
cmake --build --preset release
# Output: build/Release/bin/LS-NTGF-GUI.exe

# Deploy Qt DLLs (first time or after Qt update)
D:/Tools-DV/Qt/6.10.1/msvc2022_64/bin/windeployqt.exe build/Release/bin/LS-NTGF-GUI.exe
```

Uses Ninja generator (not VS generator). C++17.

## Module Dependency Flow

```
main.cpp -> main_window (MDI container, tabbed UI)
  -> parameter_widget   (problem parameter input)
  -> results_widget     (solution display, reads ls-ntgf-all JSON output)
  -> log_widget         (real-time solver log viewer)
  -> cplex_settings_widget (CPLEX parameter config)
  -> analysis_widget    (problem statistics)
  -> generator_widget   (test data generation UI)

solver_worker    (QProcess, launches ls-ntgf-all.exe)  <- KEY
generator_worker (QProcess, launches data generator)
difficulty_mapper (problem difficulty classification)

panels/ (visualization components)
  -> overview_panel, capacity_panel, setup_panel, variables_panel

widgets/ (custom chart widgets)
  -> metric_card, cost_bar, line_chart, heatmap
```

## Gotchas

- solver_worker.cpp 中有硬编码的 exe 路径回退 (D:/YM-Code/...), 应优先用相对路径
- solver_worker 通过 QTimer 500ms 轮询读取日志文件, 不是 stdout pipe
- 改 ls-ntgf-all 的 CLI 参数 -> 必须同步改 solver_worker.cpp 的参数构建逻辑
- 改 ls-ntgf-all 的 JSON 输出格式 -> 必须同步改 results_widget.cpp 的解析逻辑
- 纯 Qt 前端, 不直接链接 CPLEX
