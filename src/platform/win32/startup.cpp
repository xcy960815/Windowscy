/**
 * @file startup.cpp
 * @brief Windows 开机启动实现
 */

#ifdef _WIN32

#include "platform/win32/startup.h"

#include <windows.h>

#include <string>

namespace maccy::win32 {

namespace {

/** 注册表运行键路径 */
constexpr wchar_t kRunKeyPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
/** 注册表值名称 */
constexpr wchar_t kRunValueName[] = L"ClipLoom";
/** 兼容旧版本注册表值名称 */
constexpr wchar_t kLegacyRunValueName[] = L"MaccyWindows";

/**
 * @brief 获取当前可执行文件路径
 * @return std::wstring 当前可执行文件的完整路径
 */
std::wstring CurrentExecutablePath() {
  std::wstring path(MAX_PATH, L'\0');
  DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));

  while (length == path.size()) {
    path.resize(path.size() * 2);
    length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  }

  path.resize(length);
  return path;
}

}  // namespace

bool IsStartOnLoginEnabled() {
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
    return false;
  }

  const auto has_non_empty_value = [key](const wchar_t* value_name) {
    std::wstring value(1024, L'\0');
    DWORD value_type = REG_SZ;
    DWORD bytes = static_cast<DWORD>(value.size() * sizeof(wchar_t));
    const LONG result = RegQueryValueExW(
        key,
        value_name,
        nullptr,
        &value_type,
        reinterpret_cast<LPBYTE>(value.data()),
        &bytes);

    if (result != ERROR_SUCCESS || value_type != REG_SZ || bytes < sizeof(wchar_t)) {
      return false;
    }

    value.resize((bytes / sizeof(wchar_t)) - 1);
    return !value.empty();
  };

  const bool enabled = has_non_empty_value(kRunValueName) || has_non_empty_value(kLegacyRunValueName);
  RegCloseKey(key);
  return enabled;
}

bool SetStartOnLogin(bool enabled) {
  HKEY key = nullptr;
  if (RegCreateKeyExW(
          HKEY_CURRENT_USER,
          kRunKeyPath,
          0,
          nullptr,
          0,
          KEY_SET_VALUE,
          nullptr,
          &key,
          nullptr) != ERROR_SUCCESS) {
    return false;
  }

  LONG result = ERROR_SUCCESS;
  const auto delete_value_if_exists = [key](const wchar_t* value_name) {
    LONG remove_result = RegDeleteValueW(key, value_name);
    if (remove_result == ERROR_FILE_NOT_FOUND) {
      remove_result = ERROR_SUCCESS;
    }
    return remove_result;
  };

  if (enabled) {
    const std::wstring command = L"\"" + CurrentExecutablePath() + L"\"";
    result = RegSetValueExW(
        key,
        kRunValueName,
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(command.c_str()),
        static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
    if (result == ERROR_SUCCESS) {
      (void)delete_value_if_exists(kLegacyRunValueName);
    }
  } else {
    result = delete_value_if_exists(kRunValueName);
    if (result == ERROR_SUCCESS) {
      result = delete_value_if_exists(kLegacyRunValueName);
    }
  }

  RegCloseKey(key);
  return result == ERROR_SUCCESS;
}

}  // namespace maccy::win32

#endif  // _WIN32
