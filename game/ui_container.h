#pragma once
#include "stdafx.h"
#include "ui_node.h"
#include "ui_window.h"

namespace mark {

namespace ui {
class ui;
struct event;
class container final : public window {
public:
	struct info {
		resource::manager* rm = nullptr;
		module::cargo* container = nullptr;
		mark::ui::ui* ui = nullptr;
		vector<int> pos;
		
	};
	container(const info& info);
	void tick(tick_context&) override;
	bool click(const event& event) override;
private:
	void attach(vector<int> pos, module::base&);
	std::shared_ptr<const resource::image> m_cargo_bg;
	ui& m_ui;
	module::cargo& m_container;
};

} }
