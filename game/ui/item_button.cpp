#include "item_button.h"
#include <interface/item.h>
#include <sprite.h>
#include <update_context.h>

mark::ui::item_button::item_button(const info& info)
	: button_base(info)
	, m_thumbnail(info.thumbnail)
	, m_font(info.font)
	, m_item(*info.item)
{}

void mark::ui::item_button::update(update_context& context)
{
	this->render(context);
}

void mark::ui::item_button::render(update_context& context) const
{
	// Display item thumbnail
	let size = this->size();
	context.sprites[102].emplace_back([&] {
		sprite _;
		_.image = m_thumbnail;
		let pos = vd(this->pos());
		_.pos = pos - vd(0, (size.x - size.y) / 2.);
		_.size = static_cast<float>(std::max(size.x, size.y));
		_.world = false;
		_.centred = false;
		return _;
	}());
	// Display the number of items in the stack
	if (m_item.quantity() > 1) {
		let constexpr font_size = 7;
		let constexpr line_size = font_size + font_size / 2;
		context.render([&] {
			update_context::text_info _;
			_.text = std::to_string(m_item.quantity());
			let offset =
				m_item.quantity() >= 10 ? font_size + line_size : line_size;
			_.pos = vd(pos() + size - vi32(offset, line_size));
			_.font = m_font;
			_.size = static_cast<float>(font_size);
			_.centred = false;
			_.world = false;
			_.box = { 30, 30 };
			_.layer = 102;
			return _;
		}());
	}
}
