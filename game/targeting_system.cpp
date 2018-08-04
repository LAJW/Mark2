#include "targeting_system.h"
#include <interface/world_object.h>
#include <random.h>
#include <resource/manager.h>
#include <stdafx.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

auto target(
	const mark::vd& turret_pos,
	const std::pair<std::weak_ptr<const mark::unit::base>, mark::vd>& pair)
	-> std::optional<mark::vd>
{
	using namespace mark;
	let & [ unit_wk, offset ] = pair;
	if (let unit = unit_wk.lock()) {
		if (!unit->dead() && length(unit->pos() - turret_pos) < 1000.) {
			if (let modular =
					std::dynamic_pointer_cast<const unit::modular>(unit)) {
				if (modular->at(round(offset / 16.))) {
					return unit->pos() + rotate(offset, modular->rotation());
				}
				return {};
			}
			return unit->pos();
		}
	}
	return {};
}

void mark::targeting_system::update(update_context& context)
{
	let pos = m_parent.pos();
	if (let queue = std::get_if<queue_type>(&m_target)) {
		auto unit = [&]() -> shared_ptr<unit::damageable> {
			auto closest = m_parent.world().find<unit::damageable>(
				pos, 500.0, [&](let& unit) {
					return !unit.dead() && unit.team() != m_parent.team();
				});
			let min_it = std::min_element(
				closest.begin(), closest.end(), [&](let a, let b) {
					let dist_a = length(pos - a->pos());
					let dist_b = length(pos - b->pos());
					return dist_a < dist_b;
				});
			if (min_it != closest.end()) {
				return min_it->get();
			}
			if (queue->empty()) {
				return m_parent.world().find_one<unit::damageable>(
					pos, 1000.0, [&](let& unit) {
						return !unit.dead() && unit.team() != m_parent.team()
							&& context.random(0, 3) == 0;
					});
			}
			return nullptr;
		}();
		if (unit) {
			queue->push_front({ std::move(unit), {} });
		}
	}
	if (let queue = std::get_if<queue_type>(&m_target)) {
		while (!queue->empty() && !::target(pos, queue->front())) {
			queue->pop_front();
		}
	}
}

auto mark::targeting_system::can_shoot() const -> bool
{
	if (let pair = std::get_if<target_type>(&m_target)) {
		return pair->first;
	}
	return !std::get<queue_type>(m_target).empty();
}

auto mark::targeting_system::target() const -> std::optional<vd>
{
	if (let queue = std::get_if<queue_type>(&m_target)) {
		if (queue->empty()) {
			return {};
		}
		return ::target(m_parent.pos(), queue->front());
	}
	return std::get<target_type>(m_target).second;
}

auto mark::targeting_system::ai() const -> bool
{
	return std::holds_alternative<queue_type>(m_target);
}

void mark::targeting_system::target(vd pos)
{
	if (let pair = std::get_if<target_type>(&m_target)) {
		pair->second = pos;
	}
}

void mark::targeting_system::queue(vd pos, bool)
{
	auto unit = m_parent.world().find_one<unit::damageable>(
		pos, 100.f, [this](const unit::base& unit) {
			return !unit.dead() && unit.team() != m_parent.team();
		});
	if (unit) {
		if (!std::holds_alternative<queue_type>(m_target)) {
			m_target = queue_type();
		}
		auto& queue = std::get<queue_type>(m_target);
		if (let modular =
				std::dynamic_pointer_cast<const unit::modular>(unit)) {
			let offset = rotate(pos - unit->pos(), -modular->rotation());
			queue.push_back({ unit, offset });
		} else {
			queue.push_back({ unit, {} });
		}
	}
}

mark::targeting_system::targeting_system(interface::world_object& parent)
	: m_parent(parent)
{}

void mark::targeting_system::command(const command::any& any)
{
	if (let activate = std::get_if<command::activate>(&any)) {
		m_target = std::make_pair(true, activate->pos);
	} else if (let release = std::get_if<command::release>(&any)) {
		if (std::holds_alternative<target_type>(m_target)) {
			m_target = std::make_pair(false, release->pos);
		}
	} else if (let guide = std::get_if<command::guide>(&any)) {
		this->target(guide->pos);
	} else if (let queue = std::get_if<command::queue>(&any)) {
		this->queue(queue->pos, false);
	}
}
