#pragma once
#include <lfo.h>
#include <optional.h>
#include <stdafx.h>
#include <unit/base.h>

namespace mark {

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
	/// Critical chance property_manager
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

void bind(property_manager&, projectile_config& instance);
void bind(property_serializer&, const projectile_config& instance);

namespace unit {

class projectile final
	: public unit::base
	, private projectile_config
{
public:
	static constexpr const char* type_name = "unit_projectile";
	// projectile constructor attributes
	struct info
		: base::info
		, projectile_config
	{
		// for mouse-guided missiles
		shared_ptr<const vd> guide;
		float rotation = NAN;
		float phase = 0.f;
		float lfo = 0.f;
	};
	projectile(mark::world&, const YAML::Node&);
	projectile(const projectile::info&);
	auto dead() const -> bool override;
	void serialize(YAML::Emitter&) const override;
	void resolve_ref(
		const YAML::Node&,
		const std::unordered_map<uint64_t, weak_ptr<unit::base>>& units)
		override;
	auto radius() const -> double override { return 10.f; }

private:
	projectile(const projectile::info&, bool);
	void update(update_context& context) override;
	struct render_info
	{
		optional<update_context&> context;
		vd step;
		std::vector<vd> collisions;
		bool is_heavy_damage;
	};
	void render(const render_info& info) const;

	resource::image_ptr m_image;
	resource::image_ptr m_im_tail;
	resource::image_ptr m_im_explosion;
	std::unordered_set<not_null<interface::damageable*>> m_damaged;
	std::unordered_set<not_null<interface::damageable*>> m_knocked;
	shared_ptr<const vd> m_guide;
	float m_rotation;
	lfo m_rotation_lfo;
	bool m_dead = false;
};
} // namespace unit
} // namespace mark
