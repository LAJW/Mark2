#pragma once
#include "resource_manager.h"
#include "stdafx.h"
#include "yaml.h"
#include <any>

#define MARK_BIND(name) property_manager.bind(#name, instance.m_##name);

namespace mark {

class property_manager {
private:
	template <typename T>
	static void deserialise(
		std::any value_ref,
		const YAML::Node& node,
		resource::manager& rm)
	{
		auto& result =
			std::any_cast<std::reference_wrapper<T>>(value_ref).get();
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
	struct property_config {
		std::any value_ref;
		std::function<void(std::any, const YAML::Node&, resource::manager&)>
			deserialise;
	};

public:
	property_manager(resource::manager&);
	template <typename T>
	void bind(std::string key, T& value_ref)
	{
		static_assert(!std::is_const_v<T>, "Value must be mutable");
		property_config config;
		config.deserialise = deserialise<T>;
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void deserialise(const YAML::Node& node);

private:
	resource::manager& m_rm;
	std::unordered_map<std::string, property_config> m_properties;
};

class property_serialiser {
private:
	struct property_config {
		std::any value_ref;
		std::function<YAML::Node(std::any)> serialise;
	};

public:
	template <typename T>
	void bind(std::string key, const T& value_ref, std::any = {})
	{
		property_config config;
		config.serialise = [](std::any value_ref) {
			return YAML::Node(
				std::any_cast<std::reference_wrapper<const T>>(value_ref)
					.get());
		};
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void serialise(YAML::Emitter& out);

private:
	std::unordered_map<std::string, property_config> m_properties;
};
} // namespace mark
