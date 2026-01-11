# LS-NTGF-GUI: 批量计划优化器图形界面

> LS-NTGF-All 求解器的 Qt 6 图形前端
>
> 功能: 求解控制 | 实例生成 | 结果分析

---

## 目录

### 第一部分: 功能概述

- 1 [项目简介](#1-项目简介)
- 2 [功能模块](#2-功能模块)

### 第二部分: 界面说明

- 3 [求解 Tab](#3-求解-tab)
- 4 [生成 Tab](#4-生成-tab)
- 5 [分析 Tab](#5-分析-tab)

### 第三部分: 代码实现

- 6 [程序架构](#6-程序架构)
- 7 [核心组件](#7-核心组件)
- 8 [构建与运行](#8-构建与运行)

---

# 第一部分: 功能概述

## 1. 项目简介

### 1.1 功能定位

LS-NTGF-GUI 是 LS-NTGF-All 求解器的图形用户界面，提供:

- 可视化的参数配置和算法选择 (RF/RFO/RR)
- 实时的多阶段求解进度监控
- 算例生成与难度控制
- 求解结果的可视化分析

### 1.2 核心求解器

调用 LS-NTGF-All.exe 进行实际求解:

- RF: Relax-and-Fix 时间分解
- RFO: RF + Fix-and-Optimize 局部搜索
- RR: Relax-and-Recover 三阶段分解

### 1.3 关联项目

| 项目 | 说明 |
|:-----|:-----|
| [LS-NTGF-All](https://github.com/94yumingzhao/LS-NTGF-All) | 核心求解器 |
| [LS-NTGF-Data-Cap](https://github.com/94yumingzhao/LS-NTGF-Data-Cap) | 算例生成器 |

---

## 2. 功能模块

### 2.1 三大功能 Tab

| Tab | 功能 | 说明 |
|:---:|:-----|:-----|
| 求解 | 运行求解器 | 加载数据、选择算法、启动求解、监控进度 |
| 生成 | 创建测试算例 | 配置规模、设置难度、批量生成 |
| 分析 | 结果可视化 | 加载结果、图表展示、变量浏览 |

### 2.2 技术栈

| 项目 | 说明 |
|:----:|:-----|
| 编程语言 | C++17 |
| GUI 框架 | Qt 6.10 |
| 编译器 | MSVC 2022 |
| 构建系统 | CMake 3.24+ |

---

# 第二部分: 界面说明

## 3. 求解 Tab

### 3.1 左侧控制面板

**文件选择区**:
- 浏览按钮: 选择 CSV 数据文件
- 文件信息: 显示问题规模 (订单数/周期数/流向数/分组数)

**算法选择**:
- RF: 快速启发式
- RFO: RF + 局部优化
- RR: 三阶段分解

**参数配置**:
- CPLEX 时限
- 惩罚系数 (欠交/未满足)
- 大订单阈值

**结果摘要**:
- 各阶段目标值
- 运行时间
- MIP Gap

### 3.2 右侧面板

**CPLEX 设置**:
- 线程数
- 内存限制
- 工作目录

**实时日志**:
- 求解器输出
- 阶段进度
- 错误信息

---

## 4. 生成 Tab

### 4.1 规模配置

| 参数 | 范围 | 说明 |
|:----:|:----:|:-----|
| 订单数 | 50-500 | N_list |
| 周期数 | 10-52 | T_list |
| 流向数 | 3-10 | F_list |
| 分组数 | 3-10 | G_list |

### 4.2 难度控制

- 难度系数 (0.0-1.0)
- 自动映射到生成参数
- 预览生成配置

### 4.3 批量生成

- 生成数量
- 输出目录
- 实时进度

---

## 5. 分析 Tab

### 5.1 结果加载

- 加载求解器输出的 JSON 结果文件
- 解析变量值和指标

### 5.2 可视化面板

**概览面板**:
- 目标值分解 (生产/启动/库存/惩罚)
- 启动跨期统计
- 产能利用率

**变量浏览器**:
- Y 矩阵 (启动变量)
- L 矩阵 (跨期变量)
- X 矩阵 (生产变量)

**热力图**:
- 周期-分组 启动分布
- 周期-流向 库存分布

---

# 第三部分: 代码实现

## 6. 程序架构

### 6.1 目录结构

```
LS-NTGF-GUI/
+-- CMakeLists.txt
+-- CMakePresets.json
+-- README.md
+-- src/
    +-- main.cpp                    # 程序入口
    +-- main_window.h/cpp           # 主窗口
    +-- parameter_widget.h/cpp      # 参数配置
    +-- results_widget.h/cpp        # 结果显示
    +-- log_widget.h/cpp            # 日志输出
    +-- cplex_settings_widget.h/cpp # CPLEX 设置
    +-- solver_worker.h/cpp         # 求解器后台线程
    +-- generator_widget.h/cpp      # 实例生成控件
    +-- generator_worker.h/cpp      # 生成器后台线程
    +-- analysis_widget.h/cpp       # 结果分析控件
    +-- difficulty_mapper.h/cpp     # 难度参数映射
    +-- panels/                     # 分析子面板
    +-- widgets/                    # 自定义控件
```

---

## 7. 核心组件

### 7.1 组件职责

| 组件 | 文件 | 职责 |
|:-----|:-----|:-----|
| MainWindow | main_window.cpp | 主窗口，管理 Tab 切换 |
| ParameterWidget | parameter_widget.cpp | 算法和参数配置 |
| SolverWorker | solver_worker.cpp | 后台调用求解器进程 |
| GeneratorWidget | generator_widget.cpp | 算例生成界面 |
| GeneratorWorker | generator_worker.cpp | 后台生成算例 |
| AnalysisWidget | analysis_widget.cpp | 结果分析和可视化 |
| LogWidget | log_widget.cpp | 实时日志显示 |

### 7.2 线程模型

```
+----------------+     信号/槽     +----------------+
|   主线程 (UI)   | <------------> | 求解器线程     |
+----------------+                +----------------+
       |                                  |
       v                                  v
  界面更新                           QProcess 调用
  日志显示                          LS-NTGF-All.exe
```

### 7.3 求解器信号

| 信号 | 参数 | 时机 |
|:-----|:-----|:-----|
| DataLoaded | N, T, F, G | 数据加载完成 |
| OrdersMerged | 合并前, 合并后 | 订单合并完成 |
| StageStarted | 阶段号, 名称 | 阶段开始 |
| StageCompleted | 阶段号, 目标值, 时间, Gap | 阶段完成 |
| OptimizationFinished | 成功, 消息 | 求解结束 |
| LogMessage | 消息 | 日志输出 |

---

## 8. 构建与运行

### 8.1 环境要求

| 软件 | 版本 |
|:-----|:-----|
| Windows | 10/11 x64 |
| Visual Studio | 2022 |
| Qt | 6.10+ |
| CMake | 3.24+ |

### 8.2 Qt 配置

```cmake
set(CMAKE_PREFIX_PATH "D:/Tools-DV/Qt/6.10.1/msvc2022_64")
```

### 8.3 构建命令

```bash
cmake --preset vs2022-release
cmake --build build/vs2022 --config Release
```

### 8.4 部署

```bash
D:\Tools-DV\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe build/vs2022/bin/Release/LS-NTGF-GUI.exe
```

### 8.5 运行要求

- LS-NTGF-All.exe 在同目录或 PATH 中
- Qt 运行时库已部署

---

**文档版本**: 1.0
**更新日期**: 2026-01-11
