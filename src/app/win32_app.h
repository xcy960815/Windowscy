/**
 * @file win32_app.h
 * @brief Windows 应用程序主类定义
 */
#pragma once

#ifdef _WIN32

#include <windows.h>

#include <filesystem>
#include <string>
#include <vector>

#include "core/double_click_modifier.h"
#include "core/history_store.h"
#include "core/settings.h"

namespace maccy {

/**
 * @brief Windows 应用程序主类
 * @details 负责管理窗口、菜单、系统托盘、剪贴板监控等核心功能
 */
class Win32App {
 public:
  /**
   * @brief 运行应用程序
   * @param instance 应用程序实例句柄
   * @param show_command 显示命令
   * @return int 退出代码
   */
  int Run(HINSTANCE instance, int show_command);

 private:
  // ========== 常量定义 ==========

  static constexpr UINT kTrayIconId = 1;                    /**< 托盘图标 ID */
  static constexpr int kToggleHotKeyId = 1;                 /**< 切换热键 ID */
  static constexpr UINT kTrayMessage = WM_APP + 1;          /**< 托盘消息 */
  static constexpr UINT kMenuSettings = 1000;               /**< 设置菜单 ID */
  static constexpr UINT kMenuShowHistory = 1001;            /**< 显示历史菜单 ID */
  static constexpr UINT kMenuPauseCapture = 1002;           /**< 暂停捕获菜单 ID */
  static constexpr UINT kMenuIgnoreNextCopy = 1003;         /**< 忽略下次复制菜单 ID */
  static constexpr UINT kMenuClearHistory = 1004;           /**< 清除历史菜单 ID */
  static constexpr UINT kMenuClearAllHistory = 1005;        /**< 清除所有历史菜单 ID */
  static constexpr UINT kMenuToggleAutoPaste = 1006;        /**< 自动粘贴切换菜单 ID */
  static constexpr UINT kMenuTogglePlainTextPaste = 1007;   /**< 纯文本粘贴切换菜单 ID */
  static constexpr UINT kMenuTogglePreview = 1008;           /**< 预览切换菜单 ID */
  static constexpr UINT kMenuStartOnLogin = 1009;            /**< 开机启动菜单 ID */
  static constexpr UINT kMenuToggleShowSearch = 1010;       /**< 显示搜索切换菜单 ID */
  static constexpr UINT kMenuToggleRememberPosition = 1011; /**< 记住位置切换菜单 ID */
  static constexpr UINT kMenuToggleIgnoreAll = 1012;         /**< 忽略所有切换菜单 ID */
  static constexpr UINT kMenuToggleCaptureText = 1013;       /**< 捕获文本切换菜单 ID */
  static constexpr UINT kMenuToggleCaptureHtml = 1014;       /**< 捕获 HTML 切换菜单 ID */
  static constexpr UINT kMenuToggleCaptureRtf = 1015;         /**< 捕获 RTF 切换菜单 ID */
  static constexpr UINT kMenuToggleCaptureImages = 1016;     /**< 捕获图片切换菜单 ID */
  static constexpr UINT kMenuToggleCaptureFiles = 1017;      /**< 捕获文件切换菜单 ID */
  static constexpr UINT kMenuSearchModeExact = 1020;        /**< 精确搜索模式菜单 ID */
  static constexpr UINT kMenuSearchModeFuzzy = 1021;        /**< 模糊搜索模式菜单 ID */
  static constexpr UINT kMenuSearchModeRegexp = 1022;       /**< 正则搜索模式菜单 ID */
  static constexpr UINT kMenuSearchModeMixed = 1023;        /**< 混合搜索模式菜单 ID */
  static constexpr UINT kMenuSortLastCopied = 1030;          /**< 按最后复制排序菜单 ID */
  static constexpr UINT kMenuSortFirstCopied = 1031;         /**< 按首次复制排序菜单 ID */
  static constexpr UINT kMenuSortCopyCount = 1032;           /**< 按复制次数排序菜单 ID */
  static constexpr UINT kMenuPinTop = 1040;                  /**< 固定在顶部菜单 ID */
  static constexpr UINT kMenuPinBottom = 1041;               /**< 固定在底部菜单 ID */
  static constexpr UINT kMenuHistoryLimit50 = 1050;         /**< 历史限制 50 菜单 ID */
  static constexpr UINT kMenuHistoryLimit100 = 1051;         /**< 历史限制 100 菜单 ID */
  static constexpr UINT kMenuHistoryLimit200 = 1052;         /**< 历史限制 200 菜单 ID */
  static constexpr UINT kMenuHistoryLimit500 = 1053;         /**< 历史限制 500 菜单 ID */
  static constexpr UINT kMenuExit = 1099;                     /**< 退出菜单 ID */
  static constexpr int kSearchEditHeight = 28;               /**< 搜索编辑框高度 */
  static constexpr int kPopupPadding = 8;                    /**< 弹窗内边距 */
  static constexpr int kPreviewMinWidth = 200;              /**< 预览最小宽度 */
  static constexpr int kPinEditorEditControlId = 2001;       /**< 固定编辑器编辑控件 ID */
  static constexpr int kPinEditorSaveButtonId = 2002;         /**< 固定编辑器保存按钮 ID */
  static constexpr int kPinEditorCancelButtonId = 2003;       /**< 固定编辑器取消按钮 ID */

  // ========== 初始化和关闭方法 ==========

  /**
   * @brief 初始化应用程序
   * @param instance 应用程序实例句柄
   * @return bool 初始化是否成功
   */
  bool Initialize(HINSTANCE instance);

  /**
   * @brief 关闭应用程序
   */
  void Shutdown();

  /**
   * @brief 获取单实例
   * @return bool 获取是否成功
   */
  bool AcquireSingleInstance();

  /**
   * @brief 释放单实例
   */
  void ReleaseSingleInstance();

  // ========== 窗口注册和创建方法 ==========

  /**
   * @brief 注册窗口类
   * @return bool 注册是否成功
   */
  bool RegisterWindowClasses();

  /**
   * @brief 创建控制器窗口
   * @return bool 创建是否成功
   */
  bool CreateControllerWindow();

  /**
   * @brief 创建弹窗
   * @return bool 创建是否成功
   */
  bool CreatePopupWindow();

  /**
   * @brief 刷新打开触发器注册
   * @return bool 刷新是否成功
   */
  bool RefreshOpenTriggerRegistration();

  /**
   * @brief 注册切换热键
   * @return bool 注册是否成功
   */
  bool RegisterToggleHotKey();

  /**
   * @brief 启动双击监控
   * @return bool 启动是否成功
   */
  bool StartDoubleClickMonitor();

  /**
   * @brief 设置托盘图标
   * @return bool 设置是否成功
   */
  bool SetupTrayIcon();

  /**
   * @brief 移除托盘图标
   */
  void RemoveTrayIcon();

  /**
   * @brief 取消注册切换热键
   */
  void UnregisterToggleHotKey();

  /**
   * @brief 停止双击监控
   */
  void StopDoubleClickMonitor();

  // ========== 菜单和设置方法 ==========

  /**
   * @brief 显示托盘菜单
   * @param anchor 锚点位置，默认为 nullptr（使用鼠标位置）
   */
  void ShowTrayMenu(const POINT* anchor = nullptr);

  /**
   * @brief 打开设置窗口
   */
  void OpenSettingsWindow();

  /**
   * @brief 关闭设置窗口
   */
  void CloseSettingsWindow();

  /**
   * @brief 应用设置窗口的更改
   * @return bool 应用是否成功
   */
  bool ApplySettingsWindowChanges();

  /**
   * @brief 同步设置窗口控件状态
   */
  void SyncSettingsWindowControls();

  /**
   * @brief 显示设置页面
   * @param page_index 页面索引
   */
  void ShowSettingsPage(int page_index);

  /**
   * @brief 设置双击修饰符选择
   * @param key 双击修饰符键
   */
  void SetSettingsDoubleClickModifierSelection(DoubleClickModifierKey key);

  // ========== 加载和保存方法 ==========

  /**
   * @brief 加载设置
   */
  void LoadSettings();

  /**
   * @brief 持久化保存设置
   */
  void PersistSettings();

  /**
   * @brief 应用存储选项
   */
  void ApplyStoreOptions();

  /**
   * @brief 加载历史记录
   */
  void LoadHistory();

  /**
   * @brief 持久化保存历史记录
   */
  void PersistHistory() const;

  /**
   * @brief 显示启动指南
   */
  void ShowStartupGuide();

  // ========== 弹窗显示和控制方法 ==========

  /**
   * @brief 切换弹窗显示状态
   */
  void TogglePopup();

  /**
   * @brief 显示弹窗
   */
  void ShowPopup();

  /**
   * @brief 隐藏弹窗
   */
  void HidePopup();

  /**
   * @brief 将弹窗定位到光标附近
   */
  void PositionPopupNearCursor();

  /**
   * @brief 从编辑框更新搜索查询
   */
  void UpdateSearchQueryFromEdit();

  /**
   * @brief 刷新弹窗列表
   */
  void RefreshPopupList();

  /**
   * @brief 更新预览
   */
  void UpdatePreview();

  /**
   * @brief 更新托盘图标
   */
  void UpdateTrayIcon();

  /**
   * @brief 保存弹窗位置
   */
  void SavePopupPlacement();

  /**
   * @brief 同步弹窗布局
   */
  void SyncPopupLayout();

  /**
   * @brief 绘制弹窗列表项
   * @param draw_item 绘制项结构体
   */
  void DrawPopupListItem(const DRAWITEMSTRUCT& draw_item);

  // ========== 选择和操作方法 ==========

  /**
   * @brief 选择可见索引
   * @param index 索引
   */
  void SelectVisibleIndex(int index);

  /**
   * @brief 选择可见项 ID
   * @param id 项 ID
   */
  void SelectVisibleItemId(std::uint64_t id);

  /**
   * @brief 获取选中的可见项 ID
   * @return std::uint64_t 选中的项 ID
   */
  std::uint64_t SelectedVisibleItemId() const;

  /**
   * @brief 切换选中项的固定状态
   */
  void ToggleSelectedPin();

  /**
   * @brief 删除选中的项
   */
  void DeleteSelectedItem();

  /**
   * @brief 清除历史记录
   * @param include_pinned 是否包含固定项
   */
  void ClearHistory(bool include_pinned);

  /**
   * @brief 打开固定编辑器
   * @param rename_only 是否只重命名
   */
  void OpenPinEditor(bool rename_only);

  /**
   * @brief 提交固定编辑器
   */
  void CommitPinEditor();

  /**
   * @brief 关闭固定编辑器
   */
  void ClosePinEditor();

  /**
   * @brief 布局固定编辑器控件
   */
  void LayoutPinEditorControls();

  /**
   * @brief 激活选中的项
   */
  void ActivateSelectedItem();

  // ========== 事件处理方法 ==========

  /**
   * @brief 处理全局键按下事件
   * @param virtual_key 虚拟键码
   */
  void HandleGlobalKeyDown(DWORD virtual_key);

  /**
   * @brief 处理全局键释放事件
   * @param virtual_key 虚拟键码
   */
  void HandleGlobalKeyUp(DWORD virtual_key);

  /**
   * @brief 处理双击修饰符触发
   */
  void HandleDoubleClickModifierTriggered();

  /**
   * @brief 处理剪贴板更新
   */
  void HandleClipboardUpdate();

  /**
   * @brief 退出应用程序
   */
  void ExitApplication();

  // ========== 路径解析方法 ==========

  /**
   * @brief 解析历史记录路径
   * @return std::filesystem::path 历史记录路径
   */
  std::filesystem::path ResolveHistoryPath() const;

  /**
   * @brief 解析设置路径
   * @return std::filesystem::path 设置路径
   */
  std::filesystem::path ResolveSettingsPath() const;

  // ========== 消息处理方法 ==========

  /**
   * @brief 处理控制器窗口消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandleControllerMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理弹窗窗口消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandlePopupMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理列表框窗口消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandleListBoxMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理搜索编辑框消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandleSearchEditMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理固定编辑器窗口消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandlePinEditorMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理设置窗口消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandleSettingsWindowMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 处理设置双击修饰符消息
   * @param window 窗口句柄
   * @param message 消息
   * @param wparam 宽参数
   * @param lparam 长参数
   * @return LRESULT 消息处理结果
   */
  LRESULT HandleSettingsDoubleClickModifierMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  // ========== 静态窗口过程 ==========

  /**
   * @brief 控制器窗口静态过程
   */
  static LRESULT CALLBACK StaticControllerWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 弹窗窗口静态过程
   */
  static LRESULT CALLBACK StaticPopupWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 列表框窗口静态过程
   */
  static LRESULT CALLBACK StaticListBoxWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 搜索编辑框静态过程
   */
  static LRESULT CALLBACK StaticSearchEditProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 固定编辑器窗口静态过程
   */
  static LRESULT CALLBACK StaticPinEditorWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 设置窗口静态过程
   */
  static LRESULT CALLBACK StaticSettingsWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 设置双击修饰符静态过程
   */
  static LRESULT CALLBACK StaticSettingsDoubleClickModifierProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 低级键盘钩子静态过程
   */
  static LRESULT CALLBACK StaticLowLevelKeyboardProc(int code, WPARAM wparam, LPARAM lparam);

  /**
   * @brief 从窗口用户数据获取 Win32App 实例
   * @param window 窗口句柄
   * @return Win32App* Win32App 实例指针
   */
  static Win32App* FromWindowUserData(HWND window);

  // ========== 成员变量 ==========

  HINSTANCE instance_ = nullptr;                                          /**< 应用程序实例 */
  HWND controller_window_ = nullptr;                                       /**< 控制器窗口句柄 */
  HWND popup_window_ = nullptr;                                            /**< 弹窗窗口句柄 */
  HWND search_edit_ = nullptr;                                             /**< 搜索编辑框句柄 */
  HWND list_box_ = nullptr;                                                /**< 列表框句柄 */
  HWND preview_edit_ = nullptr;                                           /**< 预览编辑框句柄 */
  HWND pin_editor_window_ = nullptr;                                       /**< 固定编辑器窗口句柄 */
  HWND pin_editor_edit_ = nullptr;                                        /**< 固定编辑器编辑框句柄 */
  HWND settings_window_ = nullptr;                                         /**< 设置窗口句柄 */
  HWND settings_tab_ = nullptr;                                           /**< 设置标签页句柄 */
  HWND settings_general_page_ = nullptr;                                   /**< 设置常规页面句柄 */
  HWND settings_storage_page_ = nullptr;                                  /**< 设置存储页面句柄 */
  HWND settings_appearance_page_ = nullptr;                               /**< 设置外观页面句柄 */
  HWND settings_pins_page_ = nullptr;                                     /**< 设置固定页面句柄 */
  HWND settings_ignore_page_ = nullptr;                                   /**< 设置忽略页面句柄 */
  HWND settings_advanced_page_ = nullptr;                                  /**< 设置高级页面句柄 */
  HWND settings_capture_enabled_check_ = nullptr;                        /**< 启用捕获复选框 */
  HWND settings_auto_paste_check_ = nullptr;                             /**< 自动粘贴复选框 */
  HWND settings_plain_text_check_ = nullptr;                              /**< 纯文本复选框 */
  HWND settings_start_on_login_check_ = nullptr;                          /**< 开机启动复选框 */
  HWND settings_double_click_open_check_ = nullptr;                      /**< 双击打开复选框 */
  HWND settings_double_click_modifier_input_ = nullptr;                   /**< 双击修饰符输入框 */
  HWND settings_hotkey_ctrl_check_ = nullptr;                             /**< 热键 Ctrl 复选框 */
  HWND settings_hotkey_alt_check_ = nullptr;                              /**< 热键 Alt 复选框 */
  HWND settings_hotkey_shift_check_ = nullptr;                            /**< 热键 Shift 复选框 */
  HWND settings_hotkey_win_check_ = nullptr;                               /**< 热键 Win 复选框 */
  HWND settings_hotkey_key_combo_ = nullptr;                              /**< 热键组合框 */
  HWND settings_show_search_check_ = nullptr;                              /**< 显示搜索复选框 */
  HWND settings_show_preview_check_ = nullptr;                            /**< 显示预览复选框 */
  HWND settings_remember_position_check_ = nullptr;                      /**< 记住位置复选框 */
  HWND settings_show_startup_guide_check_ = nullptr;                      /**< 显示启动指南复选框 */
  HWND settings_ignore_all_check_ = nullptr;                              /**< 忽略所有复选框 */
  HWND settings_only_listed_apps_check_ = nullptr;                       /**< 仅列出应用复选框 */
  HWND settings_capture_text_check_ = nullptr;                            /**< 捕获文本复选框 */
  HWND settings_capture_html_check_ = nullptr;                            /**< 捕获 HTML 复选框 */
  HWND settings_capture_rtf_check_ = nullptr;                             /**< 捕获 RTF 复选框 */
  HWND settings_capture_images_check_ = nullptr;                          /**< 捕获图片复选框 */
  HWND settings_capture_files_check_ = nullptr;                           /**< 捕获文件复选框 */
  HWND settings_clear_history_on_exit_check_ = nullptr;                  /**< 退出时清除历史复选框 */
  HWND settings_clear_clipboard_on_exit_check_ = nullptr;                 /**< 退出时清除剪贴板复选框 */
  HWND settings_search_mode_combo_ = nullptr;                             /**< 搜索模式组合框 */
  HWND settings_sort_order_combo_ = nullptr;                              /**< 排序顺序组合框 */
  HWND settings_pin_position_combo_ = nullptr;                            /**< 固定位置组合框 */
  HWND settings_history_limit_combo_ = nullptr;                           /**< 历史限制组合框 */
  HWND settings_ignored_apps_edit_ = nullptr;                             /**< 忽略应用编辑框 */
  HWND settings_allowed_apps_edit_ = nullptr;                             /**< 允许应用编辑框 */
  HWND settings_ignored_patterns_edit_ = nullptr;                         /**< 忽略模式编辑框 */
  HWND settings_ignored_formats_edit_ = nullptr;                          /**< 忽略格式编辑框 */
  HANDLE single_instance_mutex_ = nullptr;                                 /**< 单实例互斥体 */
  WNDPROC original_search_edit_proc_ = nullptr;                           /**< 原始搜索编辑过程 */
  WNDPROC original_list_box_proc_ = nullptr;                              /**< 原始列表框过程 */
  WNDPROC original_settings_double_click_modifier_proc_ = nullptr;         /**< 原始设置双击修饰符过程 */
  HHOOK double_click_hook_ = nullptr;                                       /**< 双击钩子句柄 */
  UINT taskbar_created_message_ = 0;                                       /**< 任务栏创建消息 */
  bool toggle_hotkey_registered_ = false;                                  /**< 切换热键是否已注册 */
  bool ignore_next_clipboard_update_ = false;                              /**< 是否忽略下次剪贴板更新 */
  bool capture_enabled_ = true;                                           /**< 是否启用捕获 */
  bool ignore_next_external_copy_ = false;                                /**< 是否忽略下次外部复制 */
  std::uint32_t active_double_click_modifier_flags_ = 0;                  /**< 当前双击修饰符标志 */
  HWND previous_foreground_window_ = nullptr;                            /**< 前一个前台窗口 */
  std::string search_query_;                                              /**< 搜索查询字符串 */
  std::vector<std::uint64_t> visible_item_ids_;                            /**< 可见项 ID 列表 */
  std::filesystem::path history_path_;                                     /**< 历史记录路径 */
  std::filesystem::path settings_path_;                                   /**< 设置路径 */
  AppSettings settings_;                                                  /**< 应用程序设置 */
  HistoryStore store_;                                                    /**< 历史记录存储 */
  DoubleClickModifierKeyDetector double_click_modifier_detector_;         /**< 双击修饰符检测器 */
  DoubleClickModifierKey settings_double_click_modifier_selection_ = DoubleClickModifierKey::kNone; /**< 设置中的双击修饰符选择 */
  std::uint64_t pin_editor_item_id_ = 0;                                 /**< 固定编辑器项 ID */
  bool pin_editor_rename_only_ = false;                                   /**< 固定编辑器是否只重命名 */
  bool use_chinese_ui_ = false;                                          /**< 是否使用中文 UI */
};

}  // namespace maccy

#endif  // _WIN32
