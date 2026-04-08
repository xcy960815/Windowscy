/**
 * @file run_win32_app.h
 * @brief Win32 应用入口轻量声明
 */
#pragma once

#ifdef _WIN32

#include <windows.h>

namespace maccy {

/**
 * @brief 运行 Win32 应用主循环
 * @param instance 应用程序实例句柄
 * @param show_command 显示命令
 * @return int 应用程序退出代码
 */
int RunWin32App(HINSTANCE instance, int show_command);

}  // namespace maccy

#endif  // _WIN32
