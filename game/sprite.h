#pragma once
#include "stdafx.h"

namespace mark {
namespace resource {
	class image;
}
struct sprite {
	std::shared_ptr<const mark::resource::image> image;
	mark::vector<double> pos;
	float size = 16.f;
	float rotation = 0.f;
	size_t frame = 0;
	sf::Color color = sf::Color::White;
};
}