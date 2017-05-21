#pragma once
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "iserializable.h"
#include "idamageable.h"
#include "property.h"
#include "vector.h"
#include "map.h"
#include "particle.h"

namespace sf {
	class Texture;
}

namespace mark {
	using image = sf::Texture;
	class sprite;
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
	class world {
	public:
		world(mark::resource::manager& resource_manager);
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
			mark::idamageable::info damage;
			mark::tick_context* context = nullptr;
			mark::segment_t segment;
			unsigned piercing = 1;
			float aoe_radius = 0.f;
		};
		auto damage(mark::world::damage_info&) -> std::pair<mark::vector<double>, bool>;

		const std::shared_ptr<const mark::image> image_bar;
		const std::shared_ptr<const mark::image> image_font;
		const std::shared_ptr<const mark::image> image_stun;
	private:
		// Collide with units and terrain
		// Returns damageable and collision position
		// If damageable is null - terrain was hit
		// If nothing was hit - position is [ NAN, NAN ]
		auto collide(const mark::segment_t&) ->
			std::pair<mark::idamageable*, mark::vector<double>>;
		auto collide(mark::vector<double> center, float radius) ->
			std::vector<std::reference_wrapper<mark::idamageable>>;
		mark::map m_map;
		std::vector<std::shared_ptr<mark::unit::base>> m_units;
		mark::resource::manager& m_resource_manager;
		std::weak_ptr<mark::unit::base> m_camera_target;
		mark::vector<double> m_camera;
		std::vector<mark::particle> m_particles;
	};
}