#include "property_manager.h"
#include "stdafx.h"
#include <iostream>

mark::property_manager::property_manager(resource::manager& rm)
	: m_rm(rm)
{
}

void mark::property_manager::deserialise(const YAML::Node& node)
{
	assert(m_rm);
	for (const auto [key, config] : m_properties) {
		config.deserialise(config.value_ref, node[key], m_rm);
	}
}

void mark::property_serialiser::serialise(YAML::Emitter& out)
{
	using namespace YAML;
	for (const auto [key, config] : m_properties) {
		out << Key << key << Value << config.serialise(config.value_ref);
	}
}
