#pragma once
#include <algorithm>
#include "adsr.h"
#include "module_base.h"

namespace mark {
	namespace curve {
		typedef float(*ptr)(float);
		inline float flat(float) {
			return 1.f;
		}
		inline float linear(float x) {
			return std::max(std::min(1.f, x), 0.f);
		}
		inline float invert(float x) {
			return linear(1 - x);
		}
		inline float sin(float x) {
			return std::sin(x * (float)M_PI * 2.f) * 0.5f + 0.5f;
		}
	}
	namespace resource {
		class manager;
		class image;
	}
	namespace module {
		class turret : public mark::module::base {
		public:
			static constexpr const char* type_name = "module_turret";

			struct info {
				// base
				mark::resource::manager* resource_manager;
				// turret max health
				float max_health = 100.f;
				// turrent current health
				float cur_health = 100.f;
				// CPU used
				float cpu = 10.f;
				// Module's weight
				float weight = 100.f;

				// turret info

				// number of shots per second
				float rate_of_fire = 1.f;
				// heat to rate of fire unary function
				mark::curve::ptr rate_of_fire_curve = mark::curve::flat;
				// turret turn speed
				float angular_velocity = 360.f;
				// number of projectiles shot at once
				uint8_t projectile_count = 2;
				// delay between launching pellets in seconds
				float burst_delay = 0.f;
				// Shoots mouse-guided projectiles
				bool guided = false;
				// cone of fire
				float cone = 0.f;
				// heat-based cone of fire curve
				mark::curve::ptr cone_curve = mark::curve::flat;
				// heat generated per shot
				float heat_per_shot = 10.f;
				// current heat
				float cur_heat = 0.f;
				// turret dimensions on the grid
				mark::vector<uint8_t> size = { 2, 2 };
				// turret current rotation (for deserializer)
				float rotation = 0.f;

				// projectile info
				float physical = 10.f;
				float energy = 10.f;
				float heat = 1.f;
				float projectile_angular_velocity = 0.f;
				float velocity = 1000.f;
				float acceleration = 100.f;
				float aoe_radius = 0.f;
				float seek_radius = 500.f;
				// maximum projectile travel distance
				float range = 2000.f;
				// critical strike chance [0, 1]
				float critical_chance = 0.1f;
				// damage multiplier for critical
				float critical_multiplier = 1.5f;
				size_t piercing = 1;
			};
			turret(mark::resource::manager&, const YAML::Node&);
			turret(mark::module::turret::info&);
			virtual void tick(mark::tick_context& context) override;
			void target(mark::vector<double> pos) override;
			void shoot(mark::vector<double> pos, bool release) override;
			auto describe() const->std::string;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_im_base;
			std::shared_ptr<const mark::resource::image> m_im_cannon;
			mark::adsr       m_adsr;

			float            m_cur_cooldown = 0;
			float            m_rate_of_fire = 1.f;
			mark::curve::ptr m_rate_of_fire_curve = mark::curve::flat;
			float            m_rotation = 0.f;
			float            m_angular_velocity = 360.f;
			uint8_t          m_projectile_count = 2;
			float            m_burst_delay = 0.f;
			bool             m_guided = false;
			float            m_cone = 0.f;
			mark::curve::ptr m_cone_curve = mark::curve::flat;
			float            m_heat_per_shot = 5.f;
			float            m_critical_chance = 0.1f;
			float            m_critical_multiplier = 1.5f;

			float            m_physical = 10.f;
			float            m_energy = 10.f;
			float            m_heat = 1.f;
			float            m_projectile_angular_velocity = 0.f;
			float            m_velocity = 1000.f;
			float            m_acceleration = 100.f;
			float            m_aoe_radius = 0.f;
			float            m_seek_radius = 500.f;
			float            m_range = 2000.f;
			size_t           m_piercing = 1;
			mark::vector<double> m_target;
			bool             m_shoot = false;
		};
	}
}