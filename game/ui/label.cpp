#include "label.h"
#include <update_context.h>
#include <ui/window.h>

mark::ui::label::label(const info& info)
	: node(info)
	, m_size(info.size)
	, m_font(info.font)
	, m_text(info.text)
	, m_font_size(info.font_size)
{
	m_pos = info.pos;
}

void mark::ui::label::update(update_context& context)
{
	context.render([&] {
		update_context::text_info _;
		_.box = vd(m_size);
		_.pos = vd(this->pos() + parent().pos());
		_.layer = 101;
		_.world = false;
		_.centred = false;
		_.text = m_text;
		_.size = m_font_size;
		_.font = m_font;
		return _;
	}());
}