#pragma once
#include "stdafx.h"
#include "interface_damageable.h"
#include "map.h"

namespace mark {
class command;
struct tick_context;
class particle;
namespace resource {
class manager;
class image;
}
namespace unit {
class base;
}
class world final {
public:
	world(resource::manager& resource_manager, bool empty = false);
	world(resource::manager&, const YAML::Node&);
	~world();
	auto resource_manager() -> resource::manager&;
	void tick(tick_context&, vector<double> screen_size);
	auto map() const -> const map&;
	auto camera() const -> vector<double> { return m_camera; }
	auto find(vector<double> pos, double radius)
		-> std::vector<std::shared_ptr<unit::base>>;
	auto find(
		vector<double> pos,
		double radius,
		const std::function<bool(const unit::base&)>& pred)
		-> std::vector<std::shared_ptr<unit::base>>;
	auto find_one(
		vector<double> pos,
		double radius,
		const std::function<bool(const unit::base&)>& pred)
		-> std::shared_ptr<unit::base>;
	void command(const command& command);
	// set target for commmands
	void target(const std::shared_ptr<unit::base>& target);
	// get target for commands
	auto target() -> std::shared_ptr<unit::base>;
	auto target() const -> std::shared_ptr<const unit::base>;
	struct damage_info {
		interface::damageable::info damage;
		tick_context* context = nullptr;
		segment_t segment;
		size_t piercing = 1;
		float aoe_radius = 0.f;
	};
	auto damage(world::damage_info&)
		-> std::pair<std::optional<vector<double>>, bool>;
	// go to the next map
	void next();
	void serialize(YAML::Emitter& out) const;

	const std::shared_ptr<const resource::image> image_bar;
	const std::shared_ptr<const resource::image> image_font;
	const std::shared_ptr<const resource::image> image_stun;
private:
	// Collide with units and terrain
	// Returns damageable and collision position
	// If damageable is null - terrain was hit
	// If nothing was hit - position is [ NAN, NAN ]
	auto collide(const segment_t&)
		-> std::pair<interface::damageable*, std::optional<vector<double>>>;
	auto collide(vector<double> center, float radius)
		-> std::vector<std::reference_wrapper<interface::damageable>>;
	mark::map m_map;
	std::vector<std::shared_ptr<unit::base>> m_units;
	resource::manager& m_resource_manager;
	std::weak_ptr<unit::base> m_camera_target;
	vector<double> m_camera;
	double m_camera_velocity = 0.;
	vector<double> m_prev_target_pos;
	double m_a = 0.;
	std::vector<particle> m_particles;
};
}