/**
 * @file source_app.h
 * @brief Windows 剪贴板源应用检测接口定义
 */
#pragma once

#ifdef _WIN32

#include <windows.h>

#include <optional>
#include <string>

namespace maccy::win32 {

/**
 * @brief 源应用程序信息结构体
 * @details 存储检测到的剪贴板来源应用程序的信息
 */
struct SourceApplicationInfo {
  std::string process_name;   /**< 进程名称 */
  std::string window_title;   /**< 窗口标题 */
  DWORD process_id = 0;       /**< 进程 ID */
  bool is_current_process = false; /**< 是否为当前进程 */
};

/**
 * @brief 检测剪贴板来源应用程序
 * @return std::optional<SourceApplicationInfo> 检测到的源应用信息
 */
[[nodiscard]] std::optional<SourceApplicationInfo> DetectClipboardSource();

}  // namespace maccy::win32

#endif  // _WIN32
