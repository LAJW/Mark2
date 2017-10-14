#pragma once
#include "stdafx.h"
#include "interface_serializable.h"
#include "interface_damageable.h"
#include "property.h"
#include "vector.h"
#include "map.h"
#include "particle.h"

namespace sf {
	class Texture;
}

namespace mark {
	struct sprite;
	class command;
	struct tick_context;
	namespace resource {
		class manager;
	}
	namespace unit {
		class base;
	}
	namespace terrain {
		class base;
	}
	class world : public interface::serializable {
	public:
		world(mark::resource::manager& resource_manager, bool empty = false);
		world(mark::resource::manager&, const YAML::Node&);
		auto resource_manager() -> mark::resource::manager&;
		void tick(mark::tick_context&, mark::vector<double> screen_size);
		auto map() const->const mark::map&;
		auto camera() const->mark::vector<double> { return m_camera; }
		auto find(mark::vector<double> pos, double radius)->std::vector<std::shared_ptr<mark::unit::base>>;
		auto find(mark::vector<double> pos, double radius, const std::function<bool(const mark::unit::base&)>& pred)
			->std::vector<std::shared_ptr<mark::unit::base>>;
		auto find_one(mark::vector<double> pos, double radius, const std::function<bool(const mark::unit::base&)>& pred)
			->std::shared_ptr<mark::unit::base>;
		void command(const mark::command& command);
		// set target for commmands
		void target(const std::shared_ptr<mark::unit::base>& target);
		// get target for commands
		auto target() -> std::shared_ptr<mark::unit::base>;
		auto target() const->std::shared_ptr<const mark::unit::base>;
		struct damage_info {
			interface::damageable::info damage;
			mark::tick_context* context = nullptr;
			mark::segment_t segment;
			size_t piercing = 1;
			float aoe_radius = 0.f;
		};
		auto damage(mark::world::damage_info&) ->
			std::pair<std::optional<mark::vector<double>>, bool>;
		// go to the next map
		void next();
		void serialize(YAML::Emitter& out) const override;

		const std::shared_ptr<const mark::resource::image> image_bar;
		const std::shared_ptr<const mark::resource::image> image_font;
		const std::shared_ptr<const mark::resource::image> image_stun;
	private:
		// Collide with units and terrain
		// Returns damageable and collision position
		// If damageable is null - terrain was hit
		// If nothing was hit - position is [ NAN, NAN ]
		auto collide(const mark::segment_t&) ->
			std::pair<interface::damageable*, std::optional<mark::vector<double>>>;
		auto collide(mark::vector<double> center, float radius) ->
			std::vector<std::reference_wrapper<interface::damageable>>;
		mark::map m_map;
		std::vector<std::shared_ptr<mark::unit::base>> m_units;
		mark::resource::manager& m_resource_manager;
		std::weak_ptr<mark::unit::base> m_camera_target;
		vector<double> m_camera;
		double m_camera_velocity = 0.;
		vector<double> m_prev_target_pos;
		double m_a = 0.;
		std::vector<mark::particle> m_particles;
	};
}