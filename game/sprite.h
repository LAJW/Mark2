#pragma once
#include "stdafx.h"
#include <numeric>

namespace mark {
struct sprite final
{
	// Display everything frame
	static constexpr let all = std::numeric_limits<size_t>::max();
	// Image of the sprite
	resource::image_ptr image;
	// position of the sprite in pixels/world coodinates
	std::variant<vd, vi32> pos;
	// Size of the sprite in pixels/world coordinates
	float size = 16.f;
	// Tilt of the sprite in degrees
	float rotation = 0.f;
	// Which frame of the sprite should be displayed
	size_t frame = 0;
	// Tint of the sprite image
	sf::Color color = sf::Color::White;
	// Should sprite transformation origin be its centre or top-left corner
	bool centred = true;
};

struct path final
{
	std::vector<vd> points;
	sf::Color color = sf::Color::White;
	bool world = true;
};

struct rectangle final
{
	vd pos;
	vd size;
	sf::Color border_color = sf::Color::White;
	sf::Color background_color = sf::Color::White;
	bool world = true;
};

struct window final
{
	/// Window position in screen coordinates
	vi32 pos;
	/// Window size in screen coordinates
	vi32 size;
	/// Window scroll position
	vi32 scroll;
	/// Sprites to draw within the window
	std::map<int, std::vector<std::variant<sprite, path, rectangle>>> sprites;
};

} // namespace mark
