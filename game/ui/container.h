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
	struct info
	{
		resource::manager* rm = nullptr;
		module::cargo* container = nullptr;
		mark::ui::ui* ui = nullptr;
		vector<int> pos;
	};
	container(const info& info);
	void update(update_context&) override;
	bool click(const event& event) override;
	auto cargo() const -> const module::cargo&;
	auto size() const -> vector<int> override;

private:
	void attach(vector<int> pos, interface::item&);
	std::shared_ptr<const resource::image> m_cargo_bg;
	ui& m_ui;
	module::cargo& m_container;
};
} // namespace ui
} // namespace mark
