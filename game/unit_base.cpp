#include "stdafx.h"
#include "unit_base.h"

auto mark::unit::base_ref::world() noexcept -> mark::world&
{ return m_world.get(); }

auto mark::unit::base_ref::world() const noexcept -> const mark::world &
{ return m_world.get(); }

mark::unit::base_ref::base_ref(mark::world& world)
	: m_world(world) { }

mark::unit::base::base(mark::world& world, vector<double> pos)
	: base_ref(world), pos(pos) { }

mark::unit::base::base(mark::world& world, const YAML::Node& node)
	: base_ref(world)
	, pos(node["pos"].as<vector<double>>())
	, team(node["team"].as<unsigned>(0)) { }

void mark::unit::base::resolve_ref(
	const YAML::Node&,
	const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>&)
{ /* no-op */ }

void mark::unit::base::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << Key << "id" << Value << reinterpret_cast<size_t>(this);
	out << Key << "team" << Value << this->team();
	out << Key << "pos" << Value << BeginMap;
	out << Key << "x" << Value << pos().x;
	out << Key << "y" << Value << pos().y;
	out << EndMap;
}
