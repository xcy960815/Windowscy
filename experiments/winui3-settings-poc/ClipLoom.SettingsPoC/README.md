# ClipLoom.SettingsPoC

这是一套准备导入 `Blank App, Packaged (WinUI 3 in Desktop)` C++/WinRT 模板的源码骨架。

当前仓库不是 Windows 环境，所以这里没有直接生成 `.sln` / `.vcxproj`。这不是遗漏，而是刻意保持仓库可在非 Windows 环境下继续工作。要在 Windows 虚拟机里运行这个 PoC，请按下面步骤导入：

## 导入步骤

1. 在 Visual Studio 2022 中新建：
   `Blank App, Packaged (WinUI 3 in Desktop)`
2. 项目名称建议用：
   `ClipLoom.SettingsPoC`
3. 保留模板自带的 `pch.*`、`.vcxproj`、`.sln`、`Package.appxmanifest`
4. 用本目录下的源码替换模板中的同名 `App` / `MainWindow` 文件
5. 新增并包含：
   - `Bridge/`
   - `ViewModels/`
   - `Views/`
6. 把仓库根目录加入 Additional Include Directories
   或保留当前相对 include 路径

## 当前范围

当前 PoC 已经包含：

- `App.xaml` / `MainWindow.xaml`
- `GeneralPage`（配置页）
- `PreviewPage`（预览模块）
- `SettingsViewModel`
- `settings_snapshot_adapter`

当前 PoC 还没有包含：

- 与主 Win32 进程的真实窗口互操作
- 从实际 `settings.dat` 读取配置
- `Apply` / `Save` 的真实持久化回写
- 主弹窗替换

## 设计目标

这个 PoC 的目标不是“先把逻辑全接完”，而是先验证两件事：

1. 用原生 WinUI 3 控件后，设置页观感是否明显优于当前 Win32 owner-draw 版本
2. `SettingsSnapshot` 是否足以作为统一桥接层，避免未来 UI 继续直接操作 Win32 控件句柄

## 相关文件

- 桥接模型：`src/core/settings_snapshot.h`
- 桥接实现：`src/core/settings_snapshot.cpp`
- PoC 接线说明：`experiments/winui3-settings-poc/README.md`

## 官方参考

- WinUI 3 / Windows App SDK 桌面应用文档  
  <https://learn.microsoft.com/windows/apps/winui/winui3/>
- Windows App SDK 部署说明  
  <https://learn.microsoft.com/windows/apps/package-and-deploy/deploy-overview>
