﻿#pragma once
#include "stdafx.h"
#include "lfo.h"
#include "module_base.h"
#include "model_shield.h"

namespace mark {
namespace module {
class shield_generator final : public base {
public:
	static constexpr const char* type_name = "shield_generator";

	shield_generator(resource::manager&, const YAML::Node&);
	shield_generator(resource::manager& resource_manager);
	auto damage(const interface::damageable::info&) -> bool override;
	auto describe() const -> std::string;
	auto collide(const segment_t&) ->
		std::optional<std::pair<
			std::reference_wrapper<interface::damageable>,
			vector<double>>> override;
	auto shield() const noexcept -> float;
	void serialise(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any& any);
private:
	void tick(tick_context& context) override;
	void render(tick_context& context) const;

	static constexpr let default_radius = 128.f;
	static constexpr let default_shield_per_energy = 10.f;

	const std::shared_ptr<const resource::image> m_im_generator;
	const float m_max_shield = 1000.f;
	const float m_radius = default_radius;
	const float m_shield_per_energy = 10.f;
	model::shield m_model_shield;
	float m_cur_shield = 1000.f;
	bool m_on = true;
};
}
}