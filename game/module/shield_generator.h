#pragma once
#include "base.h"
#include <lfo.h>
#include <model_shield.h>
#include <stdafx.h>

namespace mark {
namespace module {
class shield_generator final : public base
{
public:
	static constexpr const char* type_name = "shield_generator";

	shield_generator(resource::manager&, const YAML::Node&);
	auto damage(const interface::damageable::info&) -> bool override;
	auto describe() const -> std::string override;
	auto collide(const segment_t&)
		-> std::optional<std::pair<ref<interface::damageable>, vd>> override;
	auto shield() const noexcept -> float;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any& any) override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	// Is shield active (not stunned, broken, etc.)
	auto active() const -> bool;

	static constexpr let default_radius = 128.f;
	static constexpr let default_shield_per_energy = 10.f;

	const resource::image_ptr m_im_generator;
	float m_max_shield = 1000.f;
	float m_radius = default_radius;
	float m_shield_per_energy = 10.f;
	model::shield m_model_shield;
	float m_cur_shield = 1000.f;
	// Level (fraction) at which the shield comes back online after breaking
	// min: 0, max: 1. Value of zero means that shield should always absorb
	// damage, as long as it has at least one HP. This mechanism should prevent
	// shields from being too overpowered and being able to reliably absorb
	// large hits on low health. Default: 10%
	float m_reboot_level = .1f;
	// Is shield currently broken - did its health dropped to zero, and is
	// recharging to go back online
	bool m_broken = false;
	bool m_on = true;
};
} // namespace module
} // namespace mark
