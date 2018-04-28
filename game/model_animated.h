#pragma once
#include "stdafx.h"

namespace mark {
struct sprite;
namespace resource {
class manager;
class image;
} // namespace resource
namespace model {
class animated
{
public:
	animated(const resource::image_ptr& image);
	void update(double dt);
	auto render(
		vd pos,
		float size,
		float rotation,
		const sf::Color& color) const -> sprite;

private:
	resource::image_ptr m_image;
	float m_state = 0.f;
};
} // namespace model
} // namespace mark