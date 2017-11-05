#pragma once
#include "stdafx.h"
#include <numeric>

namespace mark {
namespace resource {
	class image;
}
struct sprite {
	// Display everything frame
	static constexpr const auto all = std::numeric_limits<size_t>::max();
	// Image of the sprite
	std::shared_ptr<const resource::image> image;
	// position of the sprite in pixels/world coodinates
	vector<double> pos;
	// Size of the sprite in pixels/world coordinates
	float size = 16.f;
	// Tilt of the sprite in degrees
	float rotation = 0.f;
	// Which frame of the sprite should be displayed
	size_t frame = 0;
	// Tint of the sprite image
	sf::Color color = sf::Color::White;
	// Should sprite use world or screen coordinates
	bool world = true;
	// Should sprite transformation origin be its centre or top-left corner
	bool centred = true;
};
}