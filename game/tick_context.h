#pragma once
#include <map>
#include <memory>
#include <vector>
#include "sprite.h"
#include "particle.h"

namespace mark {
	namespace unit {
		class base;
	}
	struct tick_context {
		double dt;
		std::map<int, std::vector<mark::sprite>> sprites;
		std::vector<std::shared_ptr<mark::unit::base>> units;
		std::vector<mark::particle> particles;
	};
	void print(std::shared_ptr<const mark::resource::image> font, std::vector<mark::sprite>& out, mark::vector<double> pos, mark::vector<double> box, float size, sf::Color color, std::string text);
}