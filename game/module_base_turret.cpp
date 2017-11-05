#include "stdafx.h"
#include "module_base_turret.h"
#include "unit_modular.h"
#include "world.h"

mark::module::base_turret::base_turret(
	vector<unsigned> size,
	const std::shared_ptr<const resource::image>& image)
	: module::base(size, image) { }
mark::module::base_turret::base_turret(
	resource::manager & rm,
	const YAML::Node & node)
	: module::base(rm, node)
	, m_target(std::make_pair(false, vector<double>(
		node["target"]["x"].as<double>(),
		node["target"]["y"].as<double>()))) { }

auto target(
	const mark::vector<double>& turret_pos,
	const std::pair<std::weak_ptr<mark::unit::base>, mark::vector<double>>& pair)
	-> std::optional<mark::vector<double>>
{
	using namespace mark;
	const auto&[unit_wk, offset] = pair;
	if (const auto unit = unit_wk.lock()) {
		if (!unit->dead() && length(unit->pos() - turret_pos) < 1000.) {
			if (const auto modular
				= std::dynamic_pointer_cast<unit::modular>(unit)) {
				if (modular->at(round(offset / 16.))) {
					return unit->pos() + rotate(offset, modular->rotation());
				}
				return { };
			}
			return unit->pos();
		}
	}
	return { };
}

void mark::module::base_turret::tick()
{
	if (const auto queue = std::get_if<queue_type>(&m_target)) {
		while (!queue->empty() && !::target(this->pos(), queue->front())) {
			queue->pop_front();
		}
	}
}

auto mark::module::base_turret::can_shoot() const -> bool
{
	if (const auto pair = std::get_if<target_type>(&m_target)) {
		return pair->first;
	}
	return !std::get<queue_type>(m_target).empty();
}

auto mark::module::base_turret::target() const -> std::optional<vector<double>>
{
	if (const auto queue = std::get_if<queue_type>(&m_target)) {
		if (queue->empty()) {
			return { };
		}
		return ::target(this->pos(), queue->front());
	}
	return std::get<target_type>(m_target).second;
}

void mark::module::base_turret::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	base::serialise(out);
	// TODO: This should serialise queue as well
	out << Key << "target" << Value << BeginMap;
	if (const auto target = this->target()) {
		out << Key << "x" << target->x;
		out << Key << "y" << target->y;
	} else {
		// TODO: This shouldn't be necessary
		out << Key << "x" << 0;
		out << Key << "y" << 0;
	}
	out << EndMap;
}

void mark::module::base_turret::target(vector<double> pos)
{ 
	if (const auto pair = std::get_if<target_type>(&m_target)) {
		pair->second = pos;
	}
}

void mark::module::base_turret::queue(vector<double> pos, bool)
{
	auto unit = parent().world().find_one<unit::damageable>(
		pos, 100.f, [this](const unit::base& unit) {
		return !unit.dead() && unit.team() != parent().team();
	});
	if (unit) {
		if (!std::holds_alternative<queue_type>(m_target)) {
			m_target = queue_type();
		}
		auto& queue = std::get<queue_type>(m_target);
		if (const auto modular
			= std::dynamic_pointer_cast<unit::modular>(unit)) {
			const auto offset = rotate(pos - unit->pos(), -modular->rotation());
			queue.push_back({ unit, offset });
		} else {
			queue.push_back({ unit, { } });
		}
	}
}

auto mark::module::base_turret::passive() const noexcept -> bool
{ return false; }

void mark::module::base_turret::command(const command::any& any)
{
	if (const auto activate = std::get_if<command::activate>(&any)) {
		m_target = std::make_pair(true, activate->pos);
	} else if (const auto release = std::get_if<command::release>(&any)) {
		if (std::holds_alternative<target_type>(m_target)) {
			m_target = std::make_pair(false, release->pos);
		}
	} else if (const auto guide = std::get_if<command::guide>(&any)) {
		this->target(guide->pos);
	} else if (const auto queue = std::get_if<command::queue>(&any)) {
		this->queue(queue->pos, false);
	}
}

