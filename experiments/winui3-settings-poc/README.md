# WinUI 3 Settings PoC

这个目录用于承载 Windows 环境下的 WinUI 3 设置页实验工程。

当前仓库已经先把“UI 桥接层”抽到了可复用的生产代码里：

- `src/core/settings_snapshot.h`
- `src/core/settings_snapshot.cpp`

这意味着后续无论使用 WinUI 3、WPF 还是继续保留 Win32，都可以通过统一的 `SettingsSnapshot` 在 UI 层读写设置，而不是直接让 UI 控件操作 `AppSettings`。

## 目标

PoC 只验证三件事：

1. WinUI 3 设置页是否明显比当前 Win32 自绘版本更好看
2. `SettingsSnapshot` 是否足以支撑 `General` / `Appearance` 页交互
3. `Windows App SDK` 的依赖、启动和包体是否在可接受范围

## 当前建议范围

首轮只做：

- `Settings`
- `Preview`

明确不做：

- 主弹窗
- 托盘菜单
- 全局热键
- 剪贴板监听
- 开机启动底层实现

## 推荐目录结构

在 Windows 开发机上建议补成下面这样：

```text
experiments/winui3-settings-poc/
  README.md
  ClipLoom.SettingsPoC.sln
  ClipLoom.SettingsPoC/
    App.xaml
    App.xaml.cpp
    MainWindow.xaml
    MainWindow.xaml.cpp
    Views/
      GeneralPage.xaml
      PreviewPage.xaml
    ViewModels/
      SettingsViewModel.h
      SettingsViewModel.cpp
    Bridge/
      settings_snapshot_adapter.h
      settings_snapshot_adapter.cpp
```

## 推荐接线方式

### 读取

- Win32 主程序读取 `AppSettings`
- 调用 `MakeSettingsSnapshot(...)`
- 把结果传给 WinUI `SettingsViewModel`

### 修改

- WinUI 控件只绑定 `SettingsViewModel`
- 点击 `Apply` 或 `Save` 时导出 `SettingsSnapshot`
- 调用 `ApplySettingsSnapshot(...)` 或 `ToAppSettings(...)`
- 最终继续复用现有 `PersistSettings()` 和相关 Win32 行为刷新逻辑

## 为什么先抽快照层

如果直接让 WinUI 3 页面读写 `AppSettings`，后面会遇到几个问题：

- UI 层和存储模型强耦合
- Apply / Save / Cancel 语义容易变乱
- 不同 UI 技术栈无法共享状态编辑流程

`SettingsSnapshot` 的作用就是把“编辑中的 UI 状态”和“已提交的应用设置”分开。

## 当前状态

- 已完成：`SettingsSnapshot` 桥接层落地并有单元测试覆盖
- 已完成：`ClipLoom.SettingsPoC/` 源码骨架，包括 `App`、`MainWindow`、`GeneralPage`、`AppearancePage`
- 未完成：WinUI 3 的 `.sln` / `.vcxproj` 仍需要在 Windows + Visual Studio 环境中从官方模板生成

## 参考文档

- `docs/ui-upgrade-options.md`
- `docs/ui-upgrade-tasklist.md`
- `docs/winui3-settings-poc.md`
