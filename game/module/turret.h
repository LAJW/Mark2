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

struct projectile_config
{
	// Names starting with m_ for backward compatibility with turret and
	// projectile members

	/// Physical damage
	float m_physical = 10.f;
	/// Antimatter damage
	float m_antimatter = 0.f;
	/// Energy damage
	float m_energy = 0.f;
	/// Heat damage
	float m_heat = 1.f;
	/// Critical chance
	float m_critical_chance = 0.1f;
	/// Critical damage multiplier
	float m_critical_multiplier = 1.5f;
	/// Projectile's base speed
	double m_velocity = 1000.f;
	/// Projectile's acceleration. Use positive values for rockets,
	/// negative values for grenades and zero for dumb-fire missiles
	float m_acceleration = 0.f;
	/// Area-of-effect damage radius. Setting to zero disables AOE
	double m_aoe_radius = 0.f;
	/// Radius of search for new targets. Setting it to a value greater than
	/// zero makes the projectile behave as a heat-seeking missile
	double m_seek_radius = 500.f;
	/// Knockback caused by projectile's impact
	double m_knockback = 0.;
	/// Number of targets a projectile can pierce. 0 means max
	size_t m_piercing = 0;
	/// Maximum distance a projectile can travel before dying
	double m_range = 2000.f;
	/// Projectile turn speed
	float m_projectile_angular_velocity = 0.f;
};

namespace module {
class turret final
	: public module::base
	, private projectile_config
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
	// Calculate rotation for the next frame
	auto rotation(double dt) const noexcept -> float;
	auto cooldown(double dt) const noexcept -> float;
	auto can_shoot() const noexcept -> bool;
	auto
	make_projectile(update_context& context, mark::world&, size_t index) const
		-> shared_ptr<unit::projectile>;
	template <typename property_manager, typename T>
	static void bind(property_manager& mgr, T& instance);
	// Get targeting system
	// Returns parent targeting system if turret is stationary
	// Returns this module's targeting system if turret is not stationary
	auto targeting_system() noexcept -> mark::targeting_system&;
	auto targeting_system() const noexcept -> const mark::targeting_system&;

	unique_ptr<mark::targeting_system> m_targeting_system;
	resource::image_ptr m_image;
	resource::image_ptr m_im_orb;
	size_t m_image_variant;
	adsr m_adsr;
	shared_ptr<vd> m_shared_target = std::make_shared<vd>();

	float m_cur_cooldown = 0;
	float m_rate_of_fire = 1.f;
	// Is this turret chargeable
	bool m_is_chargeable = false;
	// Is this turret currently charging
	bool m_is_charging = false;
	curve::ptr m_rate_of_fire_curve = curve::flat;
	float m_rotation = 0.f;
	float m_angular_velocity = 360.f;
	unsigned m_projectile_count = 0;
	float m_burst_delay = 0.f;
	bool m_guided = false;
	float m_cone = 0.f;
	curve::ptr m_cone_curve = curve::flat;
	float m_heat_per_shot = 5.f;

	// Does the projectile wobble when flying
	bool m_unstable = false;
};
} // namespace module
} // namespace mark
