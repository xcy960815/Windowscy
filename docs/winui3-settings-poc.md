# WinUI 3 设置页 PoC 方案

日期：2026-04-08

## 目标

用最小成本验证一件事：

- `WinUI 3` 能不能明显提升 ClipLoom 的设置页观感
- 引入 `Windows App SDK` 后，体积、启动、焦点和系统集成复杂度是否还能接受

PoC 不追求“一次做完”，只验证关键路径。

## 建议范围

首轮只做两个页面：

- `General`
- `Appearance`

明确不做：

- 托盘、热键、剪贴板监听、开机启动注册
- 主弹窗替换
- 安装器或正式发布接入

## 结构原则

- 继续保留现有 `src/core`
- 继续保留现有 `src/platform/win32`
- 继续保留现有 Win32 主应用入口
- WinUI 3 只作为“设置页壳层试验场”

## 推荐桥接方式

### 1. 读取路径

- Win32 主程序负责读取 `AppSettings`
- 组装成 `SettingsSnapshot`
- 把快照传给 WinUI 3 设置页

### 2. 修改路径

- WinUI 3 页面只改本地 ViewModel
- 用户点击 `Apply` 或 `Save`
- 生成新的 `SettingsSnapshot`
- 由桥接层映射回 `AppSettings`
- 继续复用现有 `PersistSettings()`、`RefreshOpenTriggerRegistration()` 等逻辑

### 3. 交互原则

- 不做双向隐式绑定回写
- 所有变更都走显式提交
- 配置失败时仍由现有 Win32 逻辑返回错误结果和回滚

## 建议项目布局

可以在 Windows 开发机上新增一个实验目录：

```text
experiments/
  winui3-settings-poc/
    README.md
    ClipLoom.SettingsPoC.sln
    ClipLoom.SettingsPoC/
      App.xaml
      App.xaml.cpp
      MainWindow.xaml
      MainWindow.xaml.cpp
      Views/
        GeneralPage.xaml
        AppearancePage.xaml
      ViewModels/
        SettingsViewModel.h
        SettingsViewModel.cpp
      Bridge/
        settings_snapshot.h
        settings_snapshot.cpp
```

## 建议最小数据模型

PoC 阶段只覆盖下面这些字段就够了：

- `popup_hotkey_modifiers`
- `popup_hotkey_virtual_key`
- `double_click_popup_enabled`
- `double_click_modifier_key`
- `capture_enabled`
- `auto_paste`
- `paste_plain_text`
- `start_on_login`
- `show_startup_guide`
- `popup.show_search`
- `popup.show_preview`
- `popup.remember_last_position`
- `pin_position`

## 成功标准

- PoC 的设置页视觉质量明显高于现有 Win32 版
- 和现有设置结构对接不需要重写业务层
- 焦点切换、快捷键录制、Apply / Save 行为可以稳定工作
- 能得到清晰的包体和依赖结论

## 失败信号

- 光是接入设置页就需要大改主消息循环
- 焦点、窗口激活或输入行为经常和托盘/热键逻辑冲突
- 包体和依赖成本超出项目接受范围

## 下一步

1. 先在 Windows 上用模板创建 WinUI 3 空项目
2. 只做 `General` 页
3. 打通 `SettingsSnapshot -> ViewModel -> Apply`
4. 记录 PoC 结论，再决定是否扩大范围
