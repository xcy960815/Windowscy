/**
 * @file main_win32.cpp
 * @brief Windows 应用程序入口点
 */

#include <windows.h>

#include "app/win32_app.h"

/**
 * @brief Windows 应用程序入口点
 * @param instance 应用程序实例句柄
 * @param 待处理的命令行参数（未使用）
 * @param show_command 显示命令
 * @return int 应用程序退出代码
 */
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command) {
  maccy::Win32App app;
  return app.Run(instance, show_command);
}
