#include "item_button.h"
#include <interface/item.h>
#include <sprite.h>
#include <ui/ui.h>
#include <ui/window.h>
#include <update_context.h>

mark::ui::item_button::item_button(const info& info)
	: button_base(info)
	, m_font(info.font)
	, m_item(*info.item)
	, m_ui(*info.ui)
	, m_origin(info.origin)
{
	let& item = *info.item;
	let length = static_cast<int>(item.size().x) * 16;
	this->on_hover.insert([=, &item](const event&) {
		// HACK: This should be based on screen resolution and updated on resize
		if (this->pos().x < 1000) {
			m_ui.tooltip(this->pos() + vi32(length, 0), &item, item.describe());
		} else {
			m_ui.tooltip(this->pos() - vi32(300, 0), &item, item.describe());
		}
		return true;
	});
}

void mark::ui::item_button::update(update_context& context)
{
	this->render(context);
}

void mark::ui::item_button::render(update_context& context) const
{
	let color = m_origin && m_ui.in_recycler(m_item)
		? sf::Color(150, 150, 150, 255)
		: sf::Color::White;
	// Display item thumbnail
	let size = this->size();
	context.sprites[102].emplace_back([&] {
		sprite _;
		_.image = m_item.thumbnail();
		_.pos = this->pos() - vi32(0, (size.x - size.y) / 2);
		_.size = static_cast<float>(std::max(size.x, size.y));
		_.centred = false;
		_.color = color;
		return _;
	}());
	// Display the number of items in the stack
	if (m_item.quantity() > 1) {
		let constexpr font_size = 7;
		let constexpr line_size = font_size + font_size / 2;
		mark::render(context.sprites[102], [&] {
			text_info _;
			_.text = std::to_string(m_item.quantity());
			let offset =
				m_item.quantity() >= 10 ? font_size + line_size : line_size;
			_.pos = pos() + size - vi32(offset, line_size);
			_.font = m_font;
			_.size = static_cast<float>(font_size);
			_.centred = false;
			_.box = { 30, 30 };
			_.color = color;
			return _;
		}());
	}
}
