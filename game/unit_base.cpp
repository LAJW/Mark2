#include "stdafx.h"
#include "unit_base.h"

mark::unit::base::base(mark::world& world, vector<double> pos)
	:pos(pos), m_world(world) { }

mark::unit::base::base(mark::world& world, const YAML::Node& node):
	m_world(world),
	pos(node["pos"].as<vector<double>>()),
	team(node["team"].as<unsigned>(0)) { }

auto mark::unit::base::world() noexcept -> mark::world &
{ return m_world.get(); }

auto mark::unit::base::world() const noexcept -> const mark::world &
{ return m_world.get(); }

void mark::unit::base::world(mark::world& world) noexcept
{ m_world = world; }

void mark::unit::base::resolve_ref(
	const YAML::Node&,
	const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>&)
{ /* no-op */ }

void mark::unit::base::serialize_base(YAML::Emitter& out) const {
	using namespace YAML;
	out << Key << "id" << Value << this->id();
	out << Key << "team" << Value << this->team();
	out << Key << "pos" << Value << BeginMap;
	out << Key << "x" << Value << pos().x;
	out << Key << "y" << Value << pos().y;
	out << EndMap;
}

