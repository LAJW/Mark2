#include "recycler.h"
#include <algorithm.h>
#include <algorithm/has_one.h>
#include <module/base.h>
#include <recycle.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/chunky_button.h>
#include <ui/item_button.h>
#include <ui/tooltip.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

namespace mark {
namespace ui {

/// Remove all buttons from the recycler except "Confirm" and "Cancel"
static void clear(recycler& window)
{
	let children = window.children();
	Expects(children.size() >= 2);
	for_each(
		std::next(children.begin(), 2), children.end(), [&](let& child) {
		Expects(window.remove(child.get()));
	});
}

recycler::recycler(const info& info)
	: chunky_window(info)
	, m_ui(*info.ui)
	, m_tooltip(*info.tooltip)
	, m_modular(*info.modular)
	, m_font(info.rm->image("font.png"))
	, m_grid(info.rm->image("inventory-grid.png"))
{
	auto& rm = *info.rm;
	auto recycle_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.pos = { 0, 770 - 46 };
		_.relative = false;
		_.size = { 130, 50 };
		_.text = "Recycle";
		return _;
	}());
	recycle_button->on_click.insert([&](let&) -> handler_result {
		// Harvest items
		std::vector<std::unique_ptr<mark::interface::item>> items;
		for (let& pos : range(m_queue.size())) {
			auto& slot = m_queue[pos];
			if (!slot.empty()) {
				items.push_back(detach(slot));
			}
			slot = {};
		}
		mark::ui::clear(*this);
		for (auto&& item : mark::recycle(rm, move(items))) {
			// TODO: Do a dry run checking that all these items can fit in the
			// modular's cargo area
			let error_code = push(m_modular, move(item));
			Ensures(success(error_code) || error_code == error::code::stacked);
		}
		return { true, {} };
	});
	Ensures(success(this->append(move(recycle_button))));
	auto cancel_recycle_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.pos = { 130, 770 - 46 };
		_.relative = false;
		_.size = { 130, 50 };
		_.text = "Cancel";
		return _;
	}());
	cancel_recycle_button->on_click.insert([&](let&) -> handler_result {
		for (let& pos : range(m_queue.size())) {
			auto& slot = m_queue[pos];
			slot = {};
		}
		mark::ui::clear(*this);
		return { true, {} };
	});
	Ensures(success(this->append(move(cancel_recycle_button))));
}

void recycler::update(update_context& context)
{
	for (let i : range(vi32(m_queue.size()))) {
		context.sprites[101].push_back([&] {
			sprite _;
			_.image = m_grid;
			_.pos = this->pos() + i * static_cast<int>(mark::module::size);
			_.centred = false;
			_.size = mark::module::size;
			return _;
		}());
	}
	this->chunky_window::update(context);
}

static auto reserved(const recycler::queue_type& queue)
{
	array2d<bool, 16, 32> reserved;
	reserved.fill(false);
	for (let & [ i, slot ] : enumerate(queue)) {
		if (slot.empty()) {
			continue;
		}
		for (let j : range(i, i + vector<size_t>(item_of(slot).size()))) {
			reserved[j] = true;
		}
	}
	return reserved;
}

auto find_empty_pos_for(
	const recycler::queue_type& queue,
	const interface::item& item) -> optional<vector<size_t>>
{
	let reserved = mark::ui::reserved(queue);
	for (auto pair : enumerate(queue)) {
		auto& [i, slot] = pair;
		let item_size = item.size();
		if (all_of(range(i, i + vector<size_t>(item_size)), [&](let pos) {
				return pos.x < queue.size().x && pos.y < queue.size().y
					&& !reserved[pos];
			})) {
			return i;
		}
	}
	return {};
}

void recycler::recycle(interface::container& container, vi32 pos) noexcept
{
	if (has_one(m_queue.data(), { container, pos })) {
		return;
	}
	auto& item = item_of(mark::slot(container, pos));
	let queue_pos = find_empty_pos_for(m_queue, item);
	if (!queue_pos) {
		return;
	}
	auto& slot = m_queue[*queue_pos];
	auto button = std::make_unique<item_button>([&] {
		item_button::info _;
		_.pos = vi32(*queue_pos * static_cast<size_t>(mark::module::size));
		_.size = item.size() * static_cast<unsigned>(mark::module::size);
		_.font = m_font;
		_.item = item;
		_.ui = m_ui;
		return _;
	}());
	auto& button_ref = *button;
	button->on_click.insert([&](const event&) -> handler_result {
		slot = {};
		// Don't do anything after this as call to this function
		// destroys all contents of the lambda we're in
		(void)this->remove(button_ref);
		return { true, {} };
	});
	button->on_hover.insert([&](const event&) -> handler_result {
		m_tooltip.set(
			vi32(*queue_pos) - vi32{ 300, 0 }, &item, item.describe());
		return { true, {} };
	});
	Ensures(success(this->append(move(button))));
	slot = { container, pos };
}

auto recycler::has(const mark::interface::item& item) const noexcept -> bool
{
	return any_of(m_queue.data(), [&](let& slot) {
		return !slot.empty() && &item_of(slot) == &item;
	});
}

} // namespace ui
} // namespace mark
