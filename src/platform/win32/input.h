/**
 * @file input.h
 * @brief Windows 输入操作接口定义
 */
#pragma once

#ifdef _WIN32

#include <windows.h>

namespace maccy::win32 {

/**
 * @brief 发送粘贴快捷键
 * @param target_window 目标窗口句柄
 * @return bool 发送是否成功
 */
[[nodiscard]] bool SendPasteShortcut(HWND target_window);

}  // namespace maccy::win32

#endif  // _WIN32
