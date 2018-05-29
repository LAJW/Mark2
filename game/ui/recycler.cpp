#include "recycler.h"
#include <algorithm.h>
#include <item/shard.h>
#include <module/base.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/chunky_button.h>
#include <ui/item_button.h>
#include <ui/tooltip.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

mark::ui::recycler::recycler(const info& info)
	: chunky_window(info)
	, m_modular(*info.modular)
	, m_tooltip(*info.tooltip)
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
	recycle_button->on_click.insert([&](let&) {
		for (let& pos : range(m_queue.size())) {
			auto& slot = m_queue[pos];
			if (!slot.empty()) {
				(void)detach(slot);
				(void)push(m_modular, std::make_unique<item::shard>(rm));
			}
			slot = {};
		}
		while (next(this->children().begin(), 2) != this->children().end()) {
			let child = next(this->children().begin(), 2);
			this->remove(**child);
		}
		return true;
	});
	this->insert(move(recycle_button));
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
		for (let& pos : range(m_queue.size())) {
			auto& slot = m_queue[pos];
			slot = {};
		}
		while (next(this->children().begin(), 2) != this->children().end()) {
			let child = next(this->children().begin(), 2);
			this->remove(**child);
		}
		return true;
	});
	this->insert(move(cancel_recycle_button));
}

void mark::ui::recycler::update(update_context& context)
{
	this->chunky_window::update(context);
}

void mark::ui::recycler::recycle(
	interface::container& container,
	vi32 pos) noexcept
{
	if (all_of(m_queue.data(), [&](let& slot) {
			return slot != mark::slot{ container, pos };
		})) {
		array2d<bool, 16, 32> reserved;
		reserved.fill(false);
		for (let & [ i, slot ] : enumerate(m_queue)) {
			if (!slot.empty()) {
				let item_size = item_of(slot).size();
				for (let j : range(i, i + vector<size_t>(item_size))) {
					reserved[j] = true;
				}
			}
		}
		for (let pair : enumerate(m_queue)) {
			auto& [i, slot] = pair;
			let& item = item_of(mark::slot(container, pos));
			let item_size = item.size();
			if (all_of(range(i, i + vector<size_t>(item_size)), [&](let pos) {
					return pos.x < m_queue.size().x && pos.y < m_queue.size().y
						&& !reserved[pos];
				})) {
				auto button = std::make_unique<item_button>(
					[&, item_size = item_size, i = i] {
						item_button::info _;
						_.pos =
							vi32(i * static_cast<size_t>(mark::module::size));
						_.size = item_size
							* static_cast<unsigned>(mark::module::size);
						_.thumbnail = item.thumbnail();
						return _;
					}());
				// auto& button_ref = *button;
				button->on_click.insert([&](const event&) {
					slot = {};
					// Don't do anything after this as call to this function
					// destroys all contents of the lambda we're in
					// Can't do this during iteration
					// this->remove(button_ref);
					return false;
				});
				button->on_hover.insert([&, i = i](const event&) {
					m_tooltip.set(vi32(i) - vi32{ 300, 0 }, item.describe());
					return true;
				});
				this->insert(move(button));
				slot = { container, pos };
				break;
			}
		}
	}
}
