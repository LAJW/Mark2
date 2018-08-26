#include "label.h"
#include <sprite.h>
#include <ui/window.h>
#include <update_context.h>

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
	let parent_pos = parent() ? parent()->pos() : vi32();
	mark::render(context.sprites[101], [&] {
		text_info _;
		_.box = vd(m_size);
		_.pos = this->pos() + parent_pos;
		_.centred = false;
		_.text = m_text;
		_.size = m_font_size;
		_.font = m_font;
		return _;
	}());
}
