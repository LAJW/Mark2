#pragma once
#include "adsr.h"
#include "command.h"
#include "interface/damageable.h"
#include "lfo.h"
#include "stdafx.h"
#include "unit/base.h"

namespace mark {
class world final {
public:
	// Small empty world with square map for testing purposes
	world(resource::manager& resource_manager);
	world(
		world_stack& stack,
		resource::manager& resource_manager,
		bool initial = true);
	world(
		world_stack& stack,
		resource::manager&,
		const YAML::Node&);
	~world();
	auto resource_manager() -> resource::manager&;
	void tick(tick_context&, vector<double> screen_size);
	auto map() const -> const map&;
	auto camera() const -> vector<double> {
		return m_camera
			+ vector<double>(m_camera_x_lfo.get(), m_camera_y_lfo.get())
				* std::pow(m_camera_adsr.get(), 3.f) * 10.;
	}

	template<typename unit_type = unit::base>
	auto find(
		const vector<double>& pos,
		const double radius,
		const std::function<bool(const unit::base&)>& pred
			= [](let&) { return true; })
		-> std::vector<std::shared_ptr<unit_type>>
	{
		std::vector<std::shared_ptr<unit_type>> out;
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (let derived
					= std::dynamic_pointer_cast<unit_type>(unit)) {
					out.push_back(std::move(derived));
				}
			}
		}
		return out;
	}

	template<typename unit_type = unit::base>
	auto find(
		const vector<double>& pos,
		const double radius,
		const std::function<bool(const unit::base&)>& pred
			= [](let&) { return true; }) const
		-> const std::vector<std::shared_ptr<const unit_type>>
	{
		std::vector<std::shared_ptr<const unit_type>> out;
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (let derived
					= std::dynamic_pointer_cast<const unit_type>(unit)) {
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
			= [](let&) { return true; })
		-> std::shared_ptr<unit_type>
	{
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (let derived
					= std::dynamic_pointer_cast<unit_type>(unit)) {
					return derived;
				}
			}
		}
		return nullptr;
	}

	template<typename unit_type = unit::base>
	auto find_one(
		const vector<double>& pos,
		const double radius,
		const std::function<bool(const unit::base&)>& pred
			= [](let&) { return true; }) const
		-> std::shared_ptr<const unit_type>
	{
		for (auto& unit : m_units) {
			if (length(unit->pos() - pos) < radius && pred(*unit)) {
				if (let derived
					= std::dynamic_pointer_cast<const unit_type>(unit)) {
					return derived;
				}
			}
		}
		return nullptr;
	}
	void command(const command::any& command);
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
		size_t piercing = 1; // Number of objects to pierce
		float aoe_radius = 0.f;
	};
	// Returns pair containing list of collision points and boolean signaling 
	// whether or not terrain was hit
	auto damage(world::damage_info)
		-> std::pair<std::vector<vector<double>>, bool>;
	// go to the next map
	void next();
	void prev();
	void serialise(YAML::Emitter& out) const;
	auto templates() const -> const std::unordered_map<std::string, YAML::Node>&;

	const std::shared_ptr<const resource::image> image_bar;
	const std::shared_ptr<const resource::image> image_font;
	const std::shared_ptr<const resource::image> image_stun;
private:
	using collision_type = std::pair<
		std::reference_wrapper<interface::damageable>, vector<double>>;
	// Collide with units and terrain
	// Returns all collisions in a line stopping at the first terrain collision
	// Returns monostate if didn't collide with anything
	auto collide(const segment_t&)
		-> std::pair<std::deque<collision_type>, std::optional<vector<double>>>;
	auto collide(vector<double> center, float radius)
		-> std::vector<std::reference_wrapper<interface::damageable>>;
	std::unique_ptr<mark::map> m_map;
	lfo m_camera_x_lfo = lfo(6.f, .5f);
	lfo m_camera_y_lfo = lfo(10.f, .0f);
	adsr m_camera_adsr = adsr(0, 1, .5f, .3f);
	std::vector<std::shared_ptr<unit::base>> m_units;
	resource::manager& m_resource_manager;
	std::weak_ptr<unit::base> m_camera_target;
	vector<double> m_camera;
	double m_camera_velocity = 0.;
	vector<double> m_prev_target_pos;
	double m_a = 0.;
	std::vector<particle> m_particles;
	world_stack* m_stack = nullptr;
};
}
