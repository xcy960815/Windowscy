#pragma once

#include "../../../../src/core/settings_snapshot.h"

namespace maccy {

/**
 * @brief 生成用于 WinUI 3 PoC 的演示设置快照
 * @details 当前实验工程还没有和主 Win32 进程直接通信，因此先用 demo 快照驱动页面。
 */
[[nodiscard]] SettingsSnapshot BuildDemoSettingsSnapshot();

}  // namespace maccy
