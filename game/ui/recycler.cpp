#include "recycler.h"
#include <algorithm.h>
#include <algorithm/diff.h>
#include <algorithm/has_one.h>
#include <module/base.h>
#include <recycle.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/modular_push.h>
#include <ui/action/set_tooltip.h>
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
	for_each(std::next(children.begin(), 2), children.end(), [&](let& child) {
		Expects(window.remove(child.get()));
	});
}

recycler::recycler(const info& info)
	: chunky_window(info)
	, m_ui(*info.ui)
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
	recycle_button->on_click.insert([&](const event&) -> handler_result {
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
		handler_result result;
		result.handled = true;
		for (auto&& item : mark::recycle(rm, move(items))) {
			result.actions.push_back(
				std::make_unique<action::modular_push>(move(item)));
		}
		return result;
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

[[nodiscard]] static auto reserved(const recycler::queue_type& queue)
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

[[nodiscard]] static optional<vector<size_t>> find_empty_pos_for(
	const recycler::queue_type& queue,
	const interface::item& item)
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

void recycler::update(update_context& context)
{
	{
		let queued_items = [&] {
			std::vector<std::pair<vector<size_t>, const interface::item&>> queued_items;
			for (let pair : enumerate(m_queue)) {
				let slot = pair.second;
				if (!slot.empty()) {
					queued_items.emplace_back(pair.first, item_of(slot));
				}
			}
			return queued_items;
		}();
		let item_buttons = [&] {
			std::vector<ref<const item_button>> item_buttons;
			let children = this->children();
			for (let& child : children) {
				if (let item_button =
						dynamic_cast<const mark::ui::item_button*>(
							&child.get())) {
					item_buttons.push_back(*item_button);
				}
			}
			return item_buttons;
		}();
		let added_and_removed =
			diff(item_buttons, queued_items, [&](let& node, let& item) {
				return item.second.equals(node.get().item());
			});
		for (let pair : added_and_removed.added) {
			let before = pair.first;
			let &item = pair.second.second;
			let pos = pair.second.first;
			// A module should have never been pushed in the first place
			auto& slot = m_queue[pos];
			auto button = std::make_unique<item_button>([&] {
				item_button::info _;
				_.pos = vi32(pos * static_cast<size_t>(mark::module::size));
				_.size = item.size() * static_cast<unsigned>(mark::module::size);
				_.font = m_font;
				_.item = item;
				_.ui = m_ui;
				return _;
			}());
			button->on_click.insert([&](const event&) -> handler_result {
				slot = {};
				return { true, {} };
			});
			button->on_hover.insert([&](const event&) -> handler_result {
				return handler_result::make(
					std::make_unique<action::set_tooltip>(
						vi32(pos) - vi32{ 300, 0 }, &item, item.describe()));
			});
			let error = before == item_buttons.end()
				? this->append(move(button))
				: this->insert(*before, move(button));
			Ensures(success(error));
		}
		for (let removed : added_and_removed.removed) {
			(void)this->remove(*removed);
		}
	}
	this->render(context);
	this->chunky_window::update(context);
}

void recycler::render(update_context& context) const
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
