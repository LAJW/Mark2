#pragma once
#include "stdafx.h"
#include "ui_node.h"
#include "ui_event.h"

namespace mark {
namespace ui {
class window;
class button final : public node {
public:
	struct info {
		// Resource manager
		std::shared_ptr<const resource::image> image = nullptr;
		// Parent window
		const window* parent = nullptr;
		// Position of top left corner relative to the parent element
		vector<int> pos;
		// Size of the button
		vector<unsigned> size;
		std::string title;
		std::shared_ptr<const resource::image> font = nullptr;
	};

	button(const info&);
	void tick(tick_context&) override;

	bool click(const event&) override;
	callback_group on_click;

	bool hover(const event&) override;
	callback_group on_hover;

	auto pos() const noexcept -> vector<int> override;
	auto size() const noexcept -> vector<int> override;
private:
	button(const info&, bool);
	void render(tick_context&);

	const window& m_parent;
	// Size of the button
	const vector<unsigned> m_size;
	const std::shared_ptr<const resource::image> m_image;
	const std::shared_ptr<const resource::image> m_font;
	std::string m_title;
	double m_opacity = .5;
	bool m_hovering = false;
};

} }