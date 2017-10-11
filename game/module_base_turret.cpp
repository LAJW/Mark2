#include "stdafx.h"
#include "module_base_turret.h"
#include "unit_modular.h"
#include "world.h"

mark::module::base_turret::base_turret(
	mark::vector<unsigned> size,
	const std::shared_ptr<const mark::resource::image>& image)
	: mark::module::base(size, image) { }

mark::module::base_turret::base_turret(
	mark::resource::manager & rm,
	const YAML::Node & node)
	: mark::module::base(rm, node)
	, m_target(
		node["target"]["x"].as<double>(),
		node["target"]["y"].as<double>()) { }

auto target(
	const mark::vector<double>& turret_pos,
	const std::pair<std::weak_ptr<mark::unit::base>, mark::vector<double>>& pair)
	-> std::optional<mark::vector<double>>
{
	const auto&[unit_wk, offset] = pair;
	if (const auto unit = unit_wk.lock()) {
		if (!unit->dead() && mark::length(unit->pos() - turret_pos) < 1000.) {
			if (const auto modular
				= std::dynamic_pointer_cast<mark::unit::modular>(unit)) {
				if (modular->at(mark::round(offset / 16.))) {
					return unit->pos() + offset;
				}
			} else {
				return { unit->pos() };
			}
		}
	}
	return { };
}

void mark::module::base_turret::tick_ai()
{
	std::optional<vector<double>> queued_target;
	while (!m_queue.empty()
		&& !(queued_target = ::target(this->pos(), m_queue.front()))) {
		m_queue.pop_front();
		m_shoot = false;
	}
	if (queued_target) {
		m_target = *queued_target;
		m_shoot = true;
	} else {
		m_shoot = m_shoot || false;
	}
}

bool mark::module::base_turret::queued()
{
	return !m_queue.empty();
}

void mark::module::base_turret::target(mark::vector<double> pos)
{ m_target = pos; }

void mark::module::base_turret::shoot(mark::vector<double> pos, bool release)
{
	m_target = pos;
	m_shoot = !release;
	m_queue.clear();
}

void mark::module::base_turret::queue(mark::vector<double> pos, bool release)
{
	auto unit = parent().world().find_one(
		pos, 100.f, [this](const unit::base& unit) {
		return !unit.dead()
			&& dynamic_cast<const interface::damageable*>(&unit)
			&& unit.team() != parent().team();
	});
	if (unit) {
		if (const auto modular
			= std::dynamic_pointer_cast<unit::modular>(unit)) {
			const auto rel = mark::rotate(pos - unit->pos(), modular->rotation());
			m_queue.push_back({ unit, rel });
		}
		else {
			m_queue.push_back({ unit, vector<double>() });
		}
	}
}

auto mark::module::base_turret::passive() const noexcept -> bool
{ return false; }

