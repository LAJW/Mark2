#include <stdafx.h>
#include "base.h"

auto mark::unit::base_ref::world() noexcept -> mark::world&
{
	return m_world.get();
}

auto mark::unit::base_ref::world() const noexcept -> const mark::world&
{
	return m_world.get();
}

mark::unit::base_ref::base_ref(mark::world& world)
	: m_world(world)
{}

mark::unit::base::base(const info& info)
	: base_ref(*info.world)
	, m_pos(info.pos)
	, m_team(info.team)
{
	assert(info.world);
}

mark::unit::base::base(mark::world& world, const YAML::Node& node)
	: base([&] {
		info info;
		info.world = &world;
		info.pos = node["pos"].as<vd>();
		info.team = node["team"].as<unsigned>(0);
		return info;
	}())
{}

void mark::unit::base::resolve_ref(
	const YAML::Node&,
	const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>&)
{ /* no-op */
}

void mark::unit::base::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "id" << Value << reinterpret_cast<size_t>(this);
	out << Key << "team" << Value << this->team();
	out << Key << "pos" << Value << BeginMap;
	out << Key << "x" << Value << pos().x;
	out << Key << "y" << Value << pos().y;
	out << EndMap;
}

auto mark::unit::base::pos() const -> vd { return m_pos; }

void mark::unit::base::pos(const vd& value) { m_pos = value; }

auto mark::unit::base::team() const -> size_t { return m_team; }

void mark::unit::base::team(size_t team) { m_team = team; }
