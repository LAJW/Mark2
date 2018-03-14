#pragma once
#include "stdafx.h"
#include <any>
#include "yaml.h"
#include "resource_manager.h"

#define MARK_BIND(name) property_manager.bind(#name, instance.m_##name);
#define MARK_BIND_DEFAULT(name, default_value) property_manager.bind(#name, instance.m_##name, default_value);

namespace mark {

template<typename T>
auto any_ref_cast(std::any any_ref) -> T&
{ return std::any_cast<std::reference_wrapper<T>>(any_ref).get(); }

template<typename T>
struct property_serialise {
	static void serialise(YAML::Emitter& out, std::any value_ref)
	{
		out << any_ref_cast<const T>(value_ref);
	}
};

template<typename T>
struct property_deserialise {
	static void deserialise(
		std::any value_ref,
		const YAML::Node& node,
		std::any default_value,
		resource::manager& rm)
	{
		any_ref_cast<T>(value_ref) = [&] {
			if (node && node.IsSequence()) {
				const auto index = rm.random(size_t(0), node.size() - 1);
				return node[index].as<T>();
			}
			if constexpr (std::is_arithmetic_v<T>) {
				if (node && node.IsMap()) {
					const auto min = node["min"].as<T>();
					const auto max = node["max"].as<T>();
					return rm.random(min, max);
				}
			}
			if (default_value.has_value()) {
				return node.as<T>(std::any_cast<T>(default_value));
			}
			return node.as<T>();
		}();
	}
};

template<>
struct property_serialise<vector<unsigned>> {
	static void serialise(YAML::Emitter& out, std::any value_ref)
	{
		using namespace YAML;
		const auto& vector = any_ref_cast<const mark::vector<unsigned>>(value_ref);
		out << BeginMap;
		out << Key << "x" << vector.x;
		out << Value << "y" << vector.y;
		out << EndMap;
	}
};

template<>
struct property_deserialise<vector<unsigned>> {
	static void deserialise(
		std::any value_ref,
		const YAML::Node& node,
		std::any default_value,
		resource::manager& rm)
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
		std::function<void(
			std::any,
			const YAML::Node&,
			std::any,
			resource::manager&)> deserialise;
	};
public:
	property_manager(resource::manager&);
	template<typename T>
	void bind(std::string key, T& value_ref, std::any default_value)
	{
		static_assert(!std::is_const_v<T>, "Value must be mutable");
		property_config config;
		config.deserialise = property_deserialise<T>::deserialise;
		config.value_ref = std::ref(value_ref);
		config.default_value = move(default_value);
		m_properties[key] = std::move(config);
	}
	template<typename T>
	void bind(std::string key, T& value_ref)
	{
		static_assert(!std::is_const_v<T>, "Value must be mutable");
		property_config config;
		config.deserialise = property_deserialise<T>::deserialise;
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void deserialise(const YAML::Node& node);
private:
	resource::manager &m_rm;
	std::unordered_map<std::string, property_config> m_properties;
};

class property_serialiser
{
private:
	struct property_config
	{
		std::any value_ref;
		std::function<void(YAML::Emitter&, std::any)> serialise;
	};
public:
	template<typename T>
	void bind(std::string key, const T& value_ref, std::any = { })
	{
		property_config config;
		config.serialise = property_serialise<T>::serialise;
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void serialise(YAML::Emitter& out);
private:
	std::unordered_map<std::string, property_config> m_properties;
};

}
