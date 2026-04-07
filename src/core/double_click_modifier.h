#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>

namespace maccy {

enum class DoubleClickModifierKey {
  kNone,
  kAlt,
  kShift,
  kControl,
};

constexpr std::uint32_t kDoubleClickModifierFlagAlt = 0x0001;
constexpr std::uint32_t kDoubleClickModifierFlagShift = 0x0002;
constexpr std::uint32_t kDoubleClickModifierFlagControl = 0x0004;
constexpr std::uint32_t kDoubleClickModifierFlagWin = 0x0008;

[[nodiscard]] std::string_view ToString(DoubleClickModifierKey key);
[[nodiscard]] DoubleClickModifierKey ParseDoubleClickModifierKey(std::string_view value);
[[nodiscard]] std::uint32_t ModifierFlagsForDoubleClickModifierKey(DoubleClickModifierKey key);
[[nodiscard]] DoubleClickModifierKey StandaloneDoubleClickModifierKey(std::uint32_t modifier_flags);

class DoubleClickModifierKeyDetector {
 public:
  using Clock = std::chrono::steady_clock;

  explicit DoubleClickModifierKeyDetector(
      std::chrono::milliseconds double_click_interval = std::chrono::milliseconds(350));

  void Reset();
  void HandleKeyDown();
  [[nodiscard]] std::optional<DoubleClickModifierKey> HandleModifierFlagsChanged(
      std::uint32_t modifier_flags,
      Clock::time_point now = Clock::now());

 private:
  std::chrono::milliseconds double_click_interval_;
  DoubleClickModifierKey active_key_ = DoubleClickModifierKey::kNone;
  std::optional<std::pair<DoubleClickModifierKey, Clock::time_point>> last_standalone_tap_;
  bool current_press_used_with_chord_ = false;
};

}  // namespace maccy
