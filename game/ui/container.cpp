#include "container.h"
#include <algorithm/range.h>
#include <module/cargo.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/event.h>
#include <ui/item_button.h>
#include <ui/ui.h>
#include <update_context.h>
#include <exception.h>

let constexpr label_height = 32;

mark::ui::container::container(const info& info)
	: window(info)
	, m_cargo_bg(info.rm->image("inventory-grid.png"))
	, m_header(info.rm->image("inventory-header.png"))
	, m_font(info.rm->image("font.png"))
	, m_ui(*info.ui)
	, m_container(*info.container)
{}

void mark::ui::container::update(update_context& context)
{
	std::vector<const mark::interface::item*> contents;
	for (let& item : m_container.items()) {
		contents.push_back(item.get());
	}
	if (contents != m_prev_contents) {
		m_prev_contents = contents;
		this->clear();
		let interior_size = m_container.interior_size();
		for (let& pos : range(interior_size)) {
			if (auto& item_ptr = m_container.items()[pos.x + pos.y * 16]) {
				auto& item = *item_ptr;
				this->attach(pos, item);
			}
		}
	}
	context.sprites[100].push_back([&] {
		sprite _;
		_.image = m_header;
		_.pos = this->pos();
		_.size = 32.f;
		_.frame = std::numeric_limits<size_t>::max();
		_.centred = false;
		return _;
	}());
	mark::render(context.sprites[100], [&] {
		text_info _;
		_.box = vd(256, 32);
		_.pos = this->pos();
		_.layer = 100;
		_.centred = false;
		_.font = m_font;
		_.text = "Container"; // TODO: Pull from the container
		_.color = sf::Color::Cyan;
		return _;
	}());
	for (let i : range(vi32(16, 4))) {
		context.sprites[100].push_back([&] {
			sprite _;
			_.image = m_cargo_bg;
			_.pos = this->pos() + i * 16 + vi32(0, 32);
			_.size = 16.f;
			_.frame = std::numeric_limits<size_t>::max();
			_.centred = false;
			return _;
		}());
	}
	this->window::update(context);
}

mark::ui::handler_result mark::ui::container::click(const event& event)
{
	auto super_result = this->window::click(event);
	if (super_result.handled) {
		return super_result;
	}
	let grabbed = m_ui.grabbed();
	if (!grabbed) {
		return { false, {} };
	}
	auto& module = *grabbed;
	let module_size = vd(module.size());
	const auto relative_pos = vd(event.cursor - this->pos() - vi32(0, label_height));
	let pos = round(
		relative_pos / static_cast<double>(mark::module::size)
		- module_size / 2.);
	if (!m_container.can_attach(pos, module)) {
		return { false, {} };
	}
	handler_result result = { true, {} };
	result.actions.push_back(
		std::make_unique<mark::ui::legacy_action>([pos, this] {
		let result = m_container.attach(pos, m_ui.drop());
		Expects(success(result) || result == error::code::stacked);
	}));
	return result;
}

const mark::module::cargo& mark::ui::container::cargo() const
{
	return m_container;
}

auto mark::ui::container::size() const -> vi32
{
	let content_size = vi32(m_container.interior_size())
		* static_cast<int>(mark::module::size);
	return { content_size.x, content_size.y + label_height };
}

void mark::ui::container::attach(vi32 pos, interface::item& item)
{
	let button_pos = pos * 16 + vi32(0, 32);
	auto button = std::make_unique<mark::ui::item_button>([&] {
		mark::ui::item_button::info _;
		_.item = item;
		_.font = m_font;
		_.size = item.size() * 16U;
		_.pos = button_pos;
		_.ui = m_ui;
		_.relative = false;
		_.origin = true;
		return _;
	}());
	button->on_click.insert([pos, this](const event& event) {
		if (let grabbed = m_ui.grabbed()) {
			// TODO: Propagate error/notify user that object cannot be put here
			if (m_container.at(pos)->can_stack(*grabbed)) {
				(void)m_container.attach(pos, m_ui.drop());
			}
			return true;
		}
		let actual_pos = m_container.pos_at(pos);
		if (actual_pos) {
			if (event.shift) {
				m_ui.recycle(m_container, *actual_pos);
			} else {
				m_ui.drag(m_container, *actual_pos);
			}
		}
		return true;
	});
	Expects(success(this->append(move(button))));
}

auto mark::ui::container::pos() const noexcept -> vi32
{
	return m_pos + parent().pos();
}
