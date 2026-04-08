# WinUI 3 Settings PoC

这个目录预留给 Windows 环境下的 WinUI 3 设置页 PoC。

当前仓库还没有把它接进主构建，原因是：

- 现阶段先验证现有 Win32 UI 是否能快速止血
- WinUI 3 仍属于实验方向
- 当前开发环境不一定具备 `Windows App SDK` 和 Visual Studio 模板

建议在 Windows 开发机上按 `docs/winui3-settings-poc.md` 的结构创建实验工程，并保持它和主程序解耦：

- 主程序继续负责系统集成
- PoC 只验证设置页 UI 壳层和设置桥接
