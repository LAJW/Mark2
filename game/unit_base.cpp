#include "unit_base.h"

auto mark::unit::base::collide(mark::vector<double> center, float radius) ->
	 std::vector<std::reference_wrapper<mark::idamageable>> {
	return { };
}

mark::unit::base::base(mark::world& world, const YAML::Node& node):
	m_world(world),
	m_pos(node["pos"]["x"].as<double>(0), node["pos"]["y"].as<double>(0)) {

}

void mark::unit::base::serialize_base(YAML::Emitter& out) const {
	using namespace YAML;
	out << Key << "pos" << Value << BeginMap;
	out << Key << "x" << Value << m_pos.x;
	out << Key << "y" << Value << m_pos.y;
	out << EndMap;
}

