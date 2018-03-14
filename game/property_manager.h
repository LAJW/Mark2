#pragma once
#include "stdafx.h"
#include <any>
#include "yaml.h"
#include "resource_manager.h"

#define MARK_BIND(name) property_manager.bind(#name, instance.m_##name);

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
		std::any value_ref, const YAML::Node& node, resource::manager& rm)
	{
		auto& result = any_ref_cast<T>(value_ref);
		result = [&] {
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
			return node.as<T>(result);
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
		resource::manager& rm)
	{
		auto& result = any_ref_cast<vector<unsigned>>(value_ref);
		result = node.as<vector<unsigned>>(result);
	}
};

class property_manager
{
private:
	struct property_config
	{
		std::any value_ref;
		std::function<void(
			std::any, const YAML::Node&, resource::manager&)> deserialise;
	};
public:
	property_manager(resource::manager&);
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
