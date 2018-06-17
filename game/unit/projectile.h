#pragma once
#include <lfo.h>
#include <module/turret.h>
#include <optional.h>
#include <stdafx.h>
#include <unit/base.h>

namespace mark {
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
