#pragma once
#include "stdafx.h"
#include "ui_node.h"
#include "ui_event.h"

namespace mark {
namespace resource {
class manager;
class image;
}

struct tick_context;
namespace ui {

class window;
class button final : public node {
public:
	struct info {
		// Resource manager
		resource::manager* rm = nullptr;
		// Parent window
		const window* parent = nullptr;
		// Position of top left corner relative to the parent element
		mark::vector<int> pos;
		// Size of the button
		mark::vector<unsigned> size;
	};

	button(const info&);
	void tick(mark::tick_context&) override;

	bool click(const event&);
	callback_group on_click;

	auto pos() const noexcept -> mark::vector<int> override;
private:
	button(const info&, bool);
	void render(mark::tick_context&);

	const window& m_parent;
	// Position relative to the parent element
	const vector<int> m_pos;
	// Size of the button
	const vector<unsigned> m_size;
	const std::shared_ptr<const resource::image> m_image;
};

} }