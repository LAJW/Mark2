﻿#include "container.h"
#include <algorithm/range.h>
#include <exception.h>
#include <module/cargo.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/drop_into_container.h>
#include <ui/action/grab_from_container.h>
#include <ui/action/recycle.h>
#include <ui/action/release_grabbed.h>
#include <ui/action/stack_into_container.h>
#include <ui/event.h>
#include <ui/item_button.h>
#include <ui/ui.h>
#include <update_context.h>

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

namespace mark {
[[nodiscard]] bool contains(vi32 target, std::pair<vi32, vi32> rect)
{
	return target.x >= rect.first.x && target.y >= rect.first.y
		&& target.x < rect.first.x + rect.second.x
		&& target.y < rect.first.y + rect.second.y;
}
} // namespace mark

mark::ui::handler_result mark::ui::container::click(const event& event)
{
	if (auto super_actions = this->window::click(event)) {
		return super_actions;
	}
	let grabbed = m_ui.grabbed();
	if (!grabbed) {
		return contains(event.cursor, { this->pos(), this->size() })
			? handled()
			: handler_result();
	}
	auto& module = *grabbed;
	let module_size = vd(module.size());
	let relative_pos = vd(event.cursor - this->pos() - vi32(0, label_height))
		/ static_cast<double>(mark::module::size);
	let pos = round(relative_pos - module_size / 2.);
	if (!m_container.can_attach(pos, module)) {
		return contains(event.cursor, { this->pos(), this->size() })
			? handled()
			: handler_result();
	}
	return make_handler_result<action::drop_into_container>(m_container, pos);
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
	button->on_click.insert(
		[pos, this, &item](const event& event) -> handler_result {
			if (let grabbed = m_ui.grabbed()) {
				if (grabbed->equals(item)) {
					return make_handler_result<action::release_grabbed>();
				}
				// TODO: Propagate error/notify user that object cannot be put
				// here
				if (!m_container.at(pos)->can_stack(*grabbed)) {
					return handled();
				}
				return make_handler_result<action::stack_into_container>(
					m_container, pos);
			}
			if (let actual_pos = m_container.pos_at(pos)) {
				if (event.shift) {
					return make_handler_result<action::recycle>(
						m_container, *actual_pos);
				}
				return make_handler_result<action::grab_from_container>(
					m_container, *actual_pos);
			}
			return handled();
		});
	Expects(success(this->append(move(button))));
}

auto mark::ui::container::pos() const noexcept -> vi32
{
	if (let parent = this->parent()) {
		return m_pos + parent->pos();
	}
	return m_pos;
}
