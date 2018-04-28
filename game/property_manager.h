#pragma once
#include "exception.h"
#include "resource_manager.h"
#include "stdafx.h"
#include "yaml.h"
#include <any>

#define MARK_BIND(name) property_manager.bind(#name, instance.m_##name);

namespace mark {

class property_manager final
{
private:
	template <typename T>
	static auto any_ref_cast(const std::any& any) -> T*
	{
		if (let wrapper = std::any_cast<ref<T>>(&any)) {
			return &wrapper->get();
		}
		return nullptr;
	}

	class iproperty
	{
	public:
		virtual ~iproperty() = default;
		virtual auto get() -> std::any = 0;
		[[nodiscard]] virtual auto set(std::any) -> std::error_code = 0;
		[[nodiscard]] virtual auto
		randomise(const YAML::Node& node, resource::manager& rm)
			-> std::error_code = 0;
	};
	template <typename T>
	class property final : public iproperty
	{
	public:
		property(T& ref)
			: m_ref(ref)
		{}
		auto get() -> std::any override { return std::ref(m_ref); }
		auto set(std::any ref) -> std::error_code override
		{
			let node = any_ref_cast<const YAML::Node>(ref);
			if (node) {
				m_ref = node->as<T>(m_ref);
				return error::code::success;
			}
			let wrapper = any_ref_cast<T>(ref);
			if (wrapper) {
				m_ref = *wrapper;
				return error::code::success;
			}
			return error::code::bad_input;
		}
		auto randomise(const YAML::Node& node, resource::manager& rm)
			-> std::error_code override
		{
			if (node && node.IsSequence()) {
				let index = rm.random(size_t(0), node.size() - 1);
				m_ref = node[index].as<T>();
				return error::code::success;
			}
			if constexpr (std::is_arithmetic_v<T>) {
				if (node && node.IsMap()) {
					let min = node["min"].as<T>();
					let max = node["max"].as<T>();
					m_ref = rm.random(min, max);
					return error::code::success;
				}
			}
			return error::code::property_not_random;
		}

	private:
		T& m_ref;
	};

public:
	property_manager(resource::manager&);
	template <typename T>
	void bind(std::string key, T& ref)
	{
		static_assert(!std::is_const_v<T>, "Value must be mutable");
		m_properties[key] = std::make_unique<property<T>>(ref);
	}
	[[nodiscard]] auto deserialize(const YAML::Node& node) -> std::error_code;
	[[nodiscard]] auto randomise(const YAML::Node& node) -> std::error_code;
	[[nodiscard]] auto randomise(const YAML::Node& node, const std::string& key)
		-> std::error_code;
	template <typename T>
	auto get(const std::string& key) const -> const T*
	{
		let it = m_properties.find(key);
		if (it == m_properties.end()) {
			return nullptr;
		}
		if (let wrapper = any_ref_cast<const T>(it->second.get())) {
			return &wrapper->get();
		}
		return nullptr;
	}

private:
	// Inherit all inheriting properties. Leaves values as default when circular
	// inheritance is detected.
	[[nodiscard]] auto update_inherited_properties(const YAML::Node& node)
		-> std::error_code;

	resource::manager& m_rm;
	std::unordered_map<std::string, std::unique_ptr<iproperty>> m_properties;
};

class property_serializer final
{
private:
	struct property_config
	{
		std::any value_ref;
		std::function<YAML::Node(std::any)> serialize;
	};

public:
	template <typename T>
	void bind(std::string key, const T& value_ref)
	{
		property_config config;
		config.serialize = [](std::any value_ref) {
			return YAML::Node(std::any_cast<cref<T>>(value_ref).get());
		};
		config.value_ref = std::ref(value_ref);
		m_properties[key] = std::move(config);
	}
	void serialize(YAML::Emitter& out);

private:
	std::unordered_map<std::string, property_config> m_properties;
};
} // namespace mark
