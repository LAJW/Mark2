#pragma once
#include "stdafx.h"
#include "interface_damageable.h"
#include "map.h"

namespace mark {
class command;
struct tick_context;
class particle;
class world_stack;
namespace resource {
class manager;
class image;
}
namespace unit {
class base;
}
class world final {
public:
	world(
		world_stack& stack,
		resource::manager& resource_manager,
		bool empty = false,
		bool initial = true);
	world(
		world_stack& stack,
		resource::manager&,
		const YAML::Node&);
	~world();
	auto resource_manager() -> resource::manager&;
	void tick(tick_context&, vector<double> screen_size);
	auto map() const -> const map&;
	auto camera() const -> vector<double> { return m_camera; }

	template<typename unit_type = unit::base>
	auto find(
		const vector<double>& pos,
		const double radius,
		const std::function<bool(const unit::base&)>& pred
			= [](const auto&) { return true; })
		-> std::vector<std::shared_ptr<unit_type>>
	{
		std::vector<std::shared_ptr<unit_type>> out;
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (const auto derived
					= std::dynamic_pointer_cast<unit_type>(unit)) {
					out.push_back(std::move(derived));
				}
			}
		}
		return out;
	}

	template<typename unit_type = unit::base>
	auto find_one(
		const vector<double>& pos,
		const double radius,
		const std::function<bool(const unit::base&)>& pred
			= [](const auto&) { return true; })
		-> std::shared_ptr<unit_type>
	{
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (const auto derived
					= std::dynamic_pointer_cast<unit_type>(unit)) {
					return derived;
				}
			}
		}
		return nullptr;
	}
	void command(const command& command);
	// set target for commmands
	void target(const std::shared_ptr<unit::base>& target);
	// get target for commands
	auto target() -> std::shared_ptr<unit::base>;
	auto target() const -> std::shared_ptr<const unit::base>;
	void attach(const std::shared_ptr<mark::unit::base>& unit);
	struct damage_info {
		interface::damageable::info damage;
		tick_context* context = nullptr;
		segment_t segment;
		size_t piercing = 1;
		float aoe_radius = 0.f;
	};
	auto damage(world::damage_info&)
		-> std::optional<std::pair<vector<double>, bool>>;
	// go to the next map
	void next();
	void prev();
	void serialise(YAML::Emitter& out) const;

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
	world_stack& m_stack;
};
}