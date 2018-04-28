#pragma once
#include "node.h"
#include "window.h"
#include <stdafx.h>

namespace mark {

namespace ui {
class ui;
struct event;
class container final : public window
{
public:
	struct info : window::info
	{
		resource::manager* rm = nullptr;
		module::cargo* container = nullptr;
		mark::ui::ui* ui = nullptr;
	};
	container(const info& info);
	void update(update_context&) override;
	bool click(const event& event) override;
	auto cargo() const -> const module::cargo&;
	auto size() const -> vi32 override;

private:
	void attach(vi32 pos, interface::item&);
	resource::image_ptr m_cargo_bg;
	ui& m_ui;
	module::cargo& m_container;
};
} // namespace ui
} // namespace mark
