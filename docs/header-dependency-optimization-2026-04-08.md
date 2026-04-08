# 头文件依赖收敛记录

日期：2026-04-08

## 目标

在不打断当前 Win32 UI 重构的前提下，先做一轮低风险的头文件优化：

- 缩小公共头的传递依赖
- 让“只需要类型”的头不再顺带包含完整实现接口
- 给 Win32 入口提供轻量声明，避免入口文件直接依赖 `win32_app.h`

## 本次改动

### 1. 抽出历史记录轻量类型头

新增：

- `src/core/history_types.h`

承载以下轻量类型：

- `PinPosition`
- `HistorySortOrder`
- `HistoryStoreOptions`

这样 `settings.h` 只需要这些枚举和配置结构时，不必再包含完整的 `history_store.h`。

### 2. 抽出搜索模式轻量类型头

新增：

- `src/core/search_mode.h`

承载以下内容：

- `SearchMode`
- `ToString(SearchMode)`

这样 `settings.h`、`search_highlight.h` 等只依赖搜索模式定义的头文件，不必再包含完整的 `search.h` 搜索接口。

### 3. 给 Win32 入口增加轻量声明

新增：

- `src/app/run_win32_app.h`

暴露的接口只有：

- `int RunWin32App(HINSTANCE instance, int show_command);`

`main_win32.cpp` 现在只依赖这个入口声明，不再直接包含体量很大的 `win32_app.h`。这一步不能彻底瘦身 `win32_app.h`，但先把最外层入口依赖切薄，属于可独立落地的第一刀。

## 影响

### 正面影响

- `settings.h` 的 include 链更轻，减少不必要的编译耦合
- `search_highlight.h` 不再依赖完整搜索接口
- Win32 入口与应用内部大类解耦一层，后续继续做 `Win32App` Public/Internal 分离时阻力更小

### 有意不做的事

这次没有直接把 `win32_app.h` 改成真正的最小 public header，原因是当前 `Win32App` 仍采用“类声明即内部状态声明”的模式，大量成员函数和成员字段分散在多个 `.cpp` 中。直接改成 PImpl 会触及更大范围的实现迁移，和正在进行中的设置页迭代容易冲突。

## 后续建议

下一阶段可以继续做两类优化：

1. `Win32App` Public/Internal 边界拆分
   - 目标是让 `win32_app.h` 只保留构造、析构、`Run()`
   - 内部状态与私有方法迁移到 `Impl` 或内部类声明

2. Win32 内部头去传递依赖
   - 让各 `win32_app_*.cpp` 显式包含自己需要的头
   - 逐步减少 `win32_app_internal.h` 的“全量总入口”属性
