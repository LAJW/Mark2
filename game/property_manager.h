#pragma once
#include "stdafx.h"
#include "yaml.h"
#include <any>

#define MARK_BIND(name) property_manager.bind(#name, instance.m_##name);
#define MARK_BIND_DEFAULT(name, default_value) property_manager.bind(#name, instance.m_##name, default_value);

namespace mark {

template<typename T>
auto any_ref_cast(std::any any_ref) -> T&
{ return std::any_cast<std::reference_wrapper<T>>(any_ref).get(); }

template<typename T>
struct property_sync {
	static void serialise(YAML::Emitter& out, std::any value_ref)
	{ out << any_ref_cast<const T>(value_ref); }
	static void deserialise(std::any value_ref, const YAML::Node& node, std::any default_value)
	{
		if (default_value.has_value()) {
			any_ref_cast<T>(value_ref) = node.as<T>(std::any_cast<T>(default_value));
		} else {
			any_ref_cast<T>(value_ref) = node.as<T>();
		}
	}
};

template<>
struct property_sync<vector<unsigned>> {
	static void serialise(YAML::Emitter& out, std::any value_ref)
	{
		using namespace YAML;
		const auto& vector = any_ref_cast<const mark::vector<unsigned>>(value_ref); 
		out << BeginMap;
		out << Key << "x" << vector.x;
		out << Value << "y" << vector.y;
		out << EndMap;
	}
	static void deserialise(std::any value_ref, const YAML::Node& node, std::any default_value)
	{
		if (default_value.has_value()) {
			const auto default_vec = std::any_cast<mark::vector<unsigned>>(default_value);
			any_ref_cast<vector<unsigned>>(value_ref) = node.as<vector<unsigned>>(default_vec);
		} else {
			any_ref_cast<vector<unsigned>>(value_ref) = node.as<vector<unsigned>>();
		}
	}
};

class property_manager
{
private:
	struct property_config
	{
		std::any value_ref;
		std::any default_value;
		std::function<void(YAML::Emitter&, std::any)> serialise;
		std::function<void(std::any, const YAML::Node&, std::any)> deserialise;
	};
public:
	template<typename T>
	void bind(std::string key, T& value_ref, std::any default_value)
	{
		using sync = property_sync<std::remove_const_t<T>>;
		property_config config;
		config.serialise = sync::serialise;
		config.deserialise = sync::deserialise;
		config.value_ref = std::ref(value_ref);
		config.default_value = move(default_value);
		m_properties[key] = std::move(config);
	}
	template<typename T>
	void bind(std::string key, T& value_ref)
	{
		using sync = property_sync<std::remove_const_t<T>>;
		property_config config;
		config.serialise = sync::serialise;
		config.deserialise = sync::deserialise;
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void deserialise(const YAML::Node& node);
	void serialise(YAML::Emitter& out);
private:
	std::unordered_map<std::string, property_config> m_properties;
};

}
