#include "property_manager.h"
#include "stdafx.h"
#include <iostream>

static auto get_inherit(const YAML::Node& node) -> std::optional<std::string>
{
	if (node && node.IsMap()) {
		let inherit = node["inherit"].as<std::string>("");
		if (inherit.empty())
			return {};
		return inherit;
	}
	return {};
}

mark::property_manager::property_manager(resource::manager& rm)
	: m_rm(rm)
{}

auto mark::property_manager::deserialise(const YAML::Node& node)
	-> std::error_code
{
	for (let & [ key, property ] : m_properties) {
		let& child = node[key];
		if (get_inherit(child)) {
			continue;
		}
		let randomise_error = property->randomise(child, m_rm);
		if (randomise_error == error::code::property_not_random) {
			if (let error = property->set(std::ref(child))) {
				return error;
			}
		}
	}
	return update_inherited_properties(node);
}

auto mark::property_manager::randomise(const YAML::Node& node)
	-> std::error_code
{
	size_t random_count = 0;
	for (let & [ key, property ] : m_properties) {
		let& child = node[key];
		if (get_inherit(child)) {
			continue;
		}
		if (error::code::success == property->randomise(child, m_rm)) {
			++random_count;
		}
	}
	if (random_count == 0) {
		return error::code::module_not_random;
	}
	return update_inherited_properties(node);
}

auto mark::property_manager::randomise(
	const YAML::Node& node,
	const std::string& key) -> std::error_code
{
	let& child = node[key];
	if (get_inherit(child)) {
		return error::code::property_not_random;
	}
	if (let error = m_properties[key]->randomise(child, m_rm)) {
		return error;
	}
	return update_inherited_properties(node);
}

auto mark::property_manager::update_inherited_properties(const YAML::Node& node)
	-> std::error_code
{
	std::unordered_map<std::string, std::string> inheritance_map;
	std::function<std::string(const std::string&)> resolve_key =
		[&](const std::string& key) {
			std::unordered_set<std::string> trail;
			decltype(resolve_key) resolve_key_impl = [&](let& key) {
				if (trail.count(key))
					return key;
				trail.insert(key);
				let it = inheritance_map.find(key);
				if (it == inheritance_map.end())
					return key;
				return resolve_key_impl(it->second);
			};
			return resolve_key_impl(key);
		};
	for (let & [ key, config ] : m_properties) {
		(void)config;
		let& child = node[key];
		if (child && child.IsMap() && child["inherit"]) {
			inheritance_map[key] = child["inherit"].as<std::string>();
		}
	}
	for (let & [ key, ignored ] : inheritance_map) {
		(void)ignored;
		let it = m_properties.find(resolve_key(key));
		if (it == m_properties.end()) {
			return error::code::property_not_found;
		}
		if (let error = m_properties[key]->set(it->second->get())) {
			return error;
		}
	}
	return error::code::success;
}

void mark::property_serialiser::serialise(YAML::Emitter& out)
{
	using namespace YAML;
	for (let & [ key, config ] : m_properties) {
		out << Key << key << Value << config.serialise(config.value_ref);
	}
}
