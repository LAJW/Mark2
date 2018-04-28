#pragma once
#include "event.h"
#include "node.h"
#include <stdafx.h>

namespace mark {
namespace ui {
class window;
class button final : public node
{
public:
	struct info : node::info
	{
		// Resource manager
		resource::image_ptr image = nullptr;
		// Parent window
		const window* parent = nullptr;
		// Position of top left corner relative to the parent element
		vi32 pos;
		// Size of the button
		vu32 size;
		std::string title;
		resource::image_ptr font = nullptr;
	};

	button(const info&);
	void update(update_context&) override;

	bool click(const event&) override;
	callback_group on_click;

	bool hover(const event&) override;
	callback_group on_hover;

	auto pos() const noexcept -> vi32 override;
	auto size() const noexcept -> vi32 override;

private:
	button(const info&, bool);
	void render(update_context&) const;

	const window& m_parent;
	// Size of the button
	const vu32 m_size;
	const resource::image_ptr m_image;
	const resource::image_ptr m_font;
	std::string m_title;
	double m_opacity = .0;
	bool m_hovering = false;
};
} // namespace ui
} // namespace mark