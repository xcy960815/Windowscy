/**
 * @file double_click_modifier.h
 * @brief 双击修饰符键检测功能定义
 */
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>

namespace maccy {

/**
 * @brief 双击修饰符键枚举
 * @details 定义了用于触发双击弹窗的修饰符键
 */
enum class DoubleClickModifierKey {
  kNone,    /**< 无修饰符 */
  kAlt,     /**< Alt 键 */
  kShift,   /**< Shift 键 */
  kControl, /**< Control 键 */
};

/** Alt 键修饰符标志 */
constexpr std::uint32_t kDoubleClickModifierFlagAlt = 0x0001;
/** Shift 键修饰符标志 */
constexpr std::uint32_t kDoubleClickModifierFlagShift = 0x0002;
/** Control 键修饰符标志 */
constexpr std::uint32_t kDoubleClickModifierFlagControl = 0x0004;
/** Win 键修饰符标志 */
constexpr std::uint32_t kDoubleClickModifierFlagWin = 0x0008;

/**
 * @brief 将双击修饰符键转换为字符串
 * @param key 双击修饰符键
 * @return std::string_view 键的字符串表示
 */
[[nodiscard]] std::string_view ToString(DoubleClickModifierKey key);

/**
 * @brief 解析双击修饰符键
 * @param value 要解析的字符串值
 * @return DoubleClickModifierKey 解析得到的修饰符键
 */
[[nodiscard]] DoubleClickModifierKey ParseDoubleClickModifierKey(std::string_view value);

/**
 * @brief 获取修饰符键对应的标志位
 * @param key 双击修饰符键
 * @return std::uint32_t 对应的标志位
 */
[[nodiscard]] std::uint32_t ModifierFlagsForDoubleClickModifierKey(DoubleClickModifierKey key);

/**
 * @brief 从修饰符标志位获取独立的双击修饰符键
 * @param modifier_flags 修饰符标志位
 * @return DoubleClickModifierKey 独立的双击修饰符键
 */
[[nodiscard]] DoubleClickModifierKey StandaloneDoubleClickModifierKey(std::uint32_t modifier_flags);

/**
 * @brief 双击修饰符键检测器类
 * @details 用于检测用户在双击操作中使用的修饰符键
 */
class DoubleClickModifierKeyDetector {
 public:
  /**
   * @brief 时钟类型别名
   */
  using Clock = std::chrono::steady_clock;

  /**
   * @brief 构造函数
   * @param double_click_interval 双击间隔时间，默认为 350 毫秒
   */
  explicit DoubleClickModifierKeyDetector(
      std::chrono::milliseconds double_click_interval = std::chrono::milliseconds(350));

  /**
   * @brief 重置检测器状态
   */
  void Reset();

  /**
   * @brief 处理键按下事件
   */
  void HandleKeyDown();

  /**
   * @brief 处理修饰符标志位变化
   * @param modifier_flags 修饰符标志位
   * @param now 当前时间点，默认为 Clock::now()
   * @return std::optional<DoubleClickModifierKey> 检测到的修饰符键
   */
  [[nodiscard]] std::optional<DoubleClickModifierKey> HandleModifierFlagsChanged(
      std::uint32_t modifier_flags,
      Clock::time_point now = Clock::now());

 private:
  std::chrono::milliseconds double_click_interval_;  /**< 双击间隔时间 */
  DoubleClickModifierKey active_key_ = DoubleClickModifierKey::kNone; /**< 当前活跃的修饰符键 */
  std::optional<std::pair<DoubleClickModifierKey, Clock::time_point>> last_standalone_tap_; /**< 上一次独立敲击 */
  bool current_press_used_with_chord_ = false; /**< 当前按键是否与和弦组合使用 */
};

}  // namespace maccy
