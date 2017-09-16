#pragma once
#include "stdafx.h"
#include "ui_node.h"
#include "ui_window.h"

namespace mark {

namespace resource {
class manager;
class image;
};

namespace module {
class cargo;
}

namespace ui {
class ui;
struct event;

class container final : public window {
public:
	struct info {
		mark::resource::manager* rm = nullptr;
		mark::module::cargo* container = nullptr;
		mark::ui::ui* ui = nullptr;
		mark::vector<int> pos;
		
	};
	container(const info& info);
	void tick(mark::tick_context&) override;
};

} }
