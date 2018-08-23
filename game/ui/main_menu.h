#pragma once
#include <stdafx.h>

namespace mark {
class mode_stack;

namespace ui {
class window;

auto make_main_menu(mark::resource::manager& rm) -> unique_ptr<window>;

} // namespace ui
} // namespace mark
