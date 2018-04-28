#pragma once
#include "base.h"
#include <adsr.h>
#include <curve.h>
#include <deque>
#include <stdafx.h>

namespace mark {
class targeting_system;
namespace unit {
class projectile;
}
namespace module {
class turret final : public module::base
{
public:
	static constexpr const char* type_name = "module_turret";

	turret(resource::manager&, const YAML::Node&);
	~turret();
	auto describe() const -> std::string override;
	void serialize(YAML::Emitter&) const override;
	auto passive() const noexcept -> bool override;
	void command(const command::any&) override;

private:
	void update(update_context& context) override;
	void render(update_context& context) const;
	auto
	make_projectile(update_context& context, mark::world&, size_t index) const
		-> std::shared_ptr<unit::projectile>;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	// Get targeting system
	// Returns parent targeting system if turret is stationary
	// Returns this module's targeting system if turret is not stationary
	auto targeting_system() -> mark::targeting_system&;

	std::unique_ptr<mark::targeting_system> m_targeting_system;
	resource::image_ptr m_image;
	resource::image_ptr m_im_orb;
	size_t m_image_variant;
	adsr m_adsr;
	std::shared_ptr<vd> m_shared_target = std::make_shared<vd>();

	float m_cur_cooldown = 0;
	float m_rate_of_fire = 1.f;
	bool m_is_chargeable = false; // Is this turret chargeable
	bool m_is_charging = false;   // Is this turret currently charging
	curve::ptr m_rate_of_fire_curve = curve::flat;
	float m_rotation = 0.f;
	float m_angular_velocity = 360.f;
	bool m_unstable = false; // Does the projectile wobble while flying
	unsigned m_projectile_count = 0;
	float m_burst_delay = 0.f;
	bool m_guided = false;
	float m_cone = 0.f;
	curve::ptr m_cone_curve = curve::flat;
	float m_heat_per_shot = 5.f;
	float m_critical_chance = 0.1f;
	float m_critical_multiplier = 1.5f;

	float m_physical = 10.f;
	float m_energy = 10.f;
	float m_heat = 1.f;
	float m_projectile_angular_velocity = 0.f;
	double m_velocity = 1000.f;
	float m_acceleration = 100.f;
	double m_aoe_radius = 0.f;
	double m_seek_radius = 500.f;
	double m_range = 2000.f;
	double m_knockback = 0.;
	size_t m_piercing = 0;
};
} // namespace module
} // namespace mark
