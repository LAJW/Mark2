#include "recycler.h"
#include <algorithm.h>
#include <algorithm/diff.h>
#include <algorithm/has_one.h>
#include <module/base.h>
#include <recycle.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/set_tooltip.h>
#include <ui/chunky_button.h>
#include <ui/item_button.h>
#include <ui/tooltip.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

namespace mark {
namespace ui {

recycler::recycler(const info& info)
	: chunky_window(info)
	, m_ui(*info.ui)
	, m_font(info.rm->image("font.png"))
	, m_queue(*info.queue)
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
	recycle_button->on_click.insert([&](const event&) {
		return handler_result::make(std::make_unique<action::legacy>(
			[&](const action::legacy::execute_info& info) {
				std::vector<std::unique_ptr<mark::interface::item>> items;
				auto& queue = *info.queue;
				for (let& pos : range(queue.size())) {
					auto& slot = queue[pos];
					if (!slot.empty()) {
						items.push_back(detach(slot));
					}
					slot = {};
				}
				for (auto&& item : mark::recycle(rm, move(items))) {
					let error_code = push(*info.modular, move(item));
					Ensures(
						success(error_code)
						|| error_code == error::code::stacked);
				}
			}));
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
	cancel_recycle_button->on_click.insert([&](let&) {
		return handler_result::make(std::make_unique<action::legacy>(
			[&](const action::legacy::execute_info& info) {
				auto& queue = *info.queue;
				for (let& pos : range(queue.size())) {
					auto& slot = queue[pos];
					slot = {};
				}
			}));
	});
	Ensures(success(this->append(move(cancel_recycle_button))));
}

void recycler::update(update_context& context)
{
	let queued_items = [&] {
		std::vector<std::pair<vector<size_t>, const interface::item&>>
			queued_items;
		for (let[pos, slot] : enumerate(m_queue)) {
			if (!slot.empty()) {
				queued_items.emplace_back(pos, item_of(slot));
			}
		}
		return queued_items;
	}();
	let item_buttons = [&] {
		std::vector<ref<const item_button>> item_buttons;
		let children = this->children();
		for (let& child : children) {
			if (let item_button =
					dynamic_cast<const mark::ui::item_button*>(&child.get())) {
				item_buttons.push_back(*item_button);
			}
		}
		return item_buttons;
	}();
	let added_and_removed =
		diff(item_buttons, queued_items, [&](let& node, let& item) {
			return item.second.equals(node.get().item());
		});
	for (let[before, pos_and_item] : added_and_removed.added) {
		let & [ pos, item ] = pos_and_item;
		auto button = std::make_unique<item_button>([&] {
			item_button::info _;
			_.pos = vi32(pos * static_cast<size_t>(mark::module::size));
			_.size = item.size() * static_cast<unsigned>(mark::module::size);
			_.font = m_font;
			_.item = item;
			_.ui = m_ui;
			return _;
		}());
		button->on_click.insert([=](const event&) -> handler_result {
			return handler_result::make(std::make_unique<action::legacy>(
				[pos](const action::base::execute_info& info) {
					(*info.queue)[pos] = {};
				}));
		});
		button->on_hover.insert([&](const event&) {
			return handler_result::make(std::make_unique<action::set_tooltip>(
				vi32(pos) - vi32{ 300, 0 }, &item, item.describe()));
		});
		let error = before == item_buttons.end()
			? this->append(move(button))
			: this->insert(*before, move(button));
		// A module should have never been pushed in the first place
		Ensures(success(error));
	}
	for (let removed : added_and_removed.removed) {
		(void)this->remove(*removed);
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

auto recycler::has(const mark::interface::item& item) const noexcept -> bool
{
	return any_of(m_queue.data(), [&](let& slot) {
		return !slot.empty() && &item_of(slot) == &item;
	});
}

} // namespace ui
} // namespace mark
