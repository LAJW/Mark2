#pragma once
#include "stdafx.h"
#include "sprite.h"
#include "particle.h"
#include "resource_manager.h"

namespace sf {
	class Sprite;
}

namespace mark {
	namespace unit {
		class base;
	}
	struct tick_context {
		tick_context(resource::manager&);
		double dt;
		std::map<int, std::vector<sprite>> sprites;
		std::map<int, std::vector<sprite>> normals;
		std::map<int, std::vector<sprite>> ui_sprites;
		std::vector<std::pair<vector<double>, sf::Color>> lights;
		std::vector<std::shared_ptr<unit::base>> units;
		std::vector<particle> particles;
		enum class bar_type {
			health,
			energy,
			shield
		};
		struct bar_info {
			std::shared_ptr<const resource::image> image;
			bar_type type;
			vector<double> pos;
			float percentage;
		};
		void render(const bar_info& info);
		// generate random number
		template<typename T>
		T random(T min, T max) {
			return m_resource_manager.random<T>(min, max);
		}
		// randomize if pair
		template<typename T>
		T random(std::pair<T, T> pair) {
			return this->random(pair.first, pair.second);
		}
		// forward return otherwise provided
		template<typename T>
		T random(T one) {
			return one;
		}
		struct spray_info {
			std::shared_ptr<const resource::image> image;
			vector<double> pos;
			float min_velocity = NAN;
			float max_velocity = NAN;
			float min_lifespan = NAN;
			float max_lifespan = NAN;
			float min_diameter = NAN;
			float max_diameter = NAN;
			size_t count = 1;
			double step = 0.f;
			float direction = 0.f;
			float cone = 360.f;
			sf::Color color = sf::Color::White;
			int layer = 0;

			void velocity(float min, float max = NAN);
			void lifespan(float min, float max = NAN);
			void diameter(float min, float max = NAN);
		};
		void render(const spray_info&);
	private:
		resource::manager& m_resource_manager;
	};
	void print(
		std::shared_ptr<const resource::image> font,
		std::vector<sprite>& out,
		vector<double> pos,
		vector<double> box,
		float size,
		sf::Color color,
		std::string text);
}