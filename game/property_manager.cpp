#include "stdafx.h"
#include "property_manager.h"
#include <iostream>

void mark::property_manager::deserialise(const YAML::Node& node)
{
	for (const auto[key, config] : m_properties) {
		config.deserialise(config.value_ref, node[key], config.default_value);
	}
}

void mark::property_manager::serialise(YAML::Emitter& out)
{
	using namespace YAML;
	for (const auto[key, config] : m_properties) {
		out << Key << key << Value;
		config.serialise(out, config.value_ref);
	}
}
