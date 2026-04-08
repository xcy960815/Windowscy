#ifdef _WIN32

#include "app/win32_app_internal.h"
#include "app/resources/resource.h"

namespace maccy {

Win32App* g_keyboard_hook_target = nullptr;

#include "app/win32_app_anon.inc"

}  // namespace maccy

#endif  // _WIN32
