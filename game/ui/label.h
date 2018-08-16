#pragma once
#include "node.h"
#include <stdafx.h>
#include <ui/event.h>

namespace mark {
namespace ui {

class window;
class label : public node
{
public:
	struct info : node::info
	{
		/// Position of top left corner relative to the parent element
		vi32 pos;
		/// Size of the label. Describes boundaries of the label
		vu32 size;
		/// Font for the label to be printed with
		resource::image_ptr font;
		/// Self-explanatory
		float font_size = 14.f;
		/// Text to be displayed on the label
		std::string text;
	};

	label(const info&);
	void update(update_context&) override;
	handler_result click(const event&) override { return { false }; }
	handler_result hover(const event&) override { return { false }; }

private:

	// Size of the label
	const vu32 m_size;
	const not_null<resource::image_ptr> m_font;
	const std::string m_text;
	float m_font_size;
};

} // namespace ui
} // namespace mark