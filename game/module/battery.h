﻿#pragma once
#include "base.h"
#include <lfo.h>
#include <stdafx.h>

namespace mark {
namespace module {
class battery final : public module::base
{
public:
	static constexpr const char* type_name = "module_battery";

	battery(resource::manager&, random& random, const YAML::Node&);
	auto describe() const -> std::string override;
	auto harvest_energy(double dt) -> float override;
	auto energy_ratio() const -> float override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;

private:
	void update(update_context& context) override;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);

	resource::image_ptr m_image_base;
	resource::image_ptr m_image_bar;
	float m_cur_energy = 0.f;
	float m_max_energy = 1000.f;
};
} // namespace module
} // namespace mark