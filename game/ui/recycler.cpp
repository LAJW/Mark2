#include "recycler.h"
#include <algorithm.h>
#include <item/shard.h>
#include <module/base.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/chunky_button.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

mark::ui::recycler::recycler(const info& info)
	: chunky_window(info)
	, m_modular(*info.modular)
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
		return true;
	});
	this->insert(move(cancel_recycle_button));
}

void mark::ui::recycler::update(update_context& context)
{
	this->chunky_window::update(context);
	for (let& [i, slot] : enumerate(m_queue)) {
		let slot_pos = vi32(i);
		if (slot.empty()) {
			continue;
		}
		context.sprites[100].emplace_back([&] {
			sprite _;
			let& item = item_of(slot);
			_.image = item.thumbnail();
			_.pos =
				vd(this->pos()
				   + (slot_pos - vi32(0, ((item.size().x - item.size().y) / 2)))
					   * static_cast<int>(module::size));
			_.size = std::max(item.size().x, item.size().y) * module::size;
			_.centred = false;
			_.world = false;
			return _;
		}());
	}
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
		for (let& [i, slot] : enumerate(m_queue)) {
			if (!slot.empty()) {
				let item_size = item_of(slot).size();
				for (let j : range(i, i + vector<size_t>(item_size))) {
					reserved[j] = true;
				}
			}
		}
		for (let pair : enumerate(m_queue)) {
			auto&[i, slot] = pair;
			let item_size = item_of(mark::slot(container, pos)).size();
			if (all_of(range(i, i + vector<size_t>(item_size)), [&](let pos) {
					return pos.x < m_queue.size().x && pos.y < m_queue.size().y
						&& !reserved[pos];
				})) {
				slot = { container, pos };
				break;
			}
		}
	}
}
