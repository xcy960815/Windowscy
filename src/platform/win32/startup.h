/**
 * @file startup.h
 * @brief Windows 开机启动接口定义
 */
#pragma once

#ifdef _WIN32

namespace maccy::win32 {

/**
 * @brief 检查是否启用了开机启动
 * @return bool 是否启用开机启动
 */
[[nodiscard]] bool IsStartOnLoginEnabled();

/**
 * @brief 设置开机启动状态
 * @param enabled 是否启用开机启动
 * @return bool 设置是否成功
 */
[[nodiscard]] bool SetStartOnLogin(bool enabled);

}  // namespace maccy::win32

#endif  // _WIN32
