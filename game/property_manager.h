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
	// Returns post-serialization hook
	static auto deserialise(
		std::any value_ref,
		const YAML::Node& node,
		const property_manager& property_manager,
		resource::manager& rm) -> std::function<void()>
	{
		auto& result =
			std::any_cast<std::reference_wrapper<T>>(value_ref).get();
		if (node && node.IsMap() && node["inherit"]) {
			let& source =
				property_manager.get<T>(node["inherit"].as<std::string>());
			return [&] { result = source; };
		}
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
		return {};
	}
	struct property_config {
		std::any value_ref;
		std::function<std::function<void()>(
			std::any,
			const YAML::Node&,
			const property_manager,
			resource::manager&)>
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
	template <typename T>
	auto get(std::string key) const -> const T&
	{
		let& any = m_properties.at(key).value_ref;
		return std::any_cast<std::reference_wrapper<T>>(any).get();
	}

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
