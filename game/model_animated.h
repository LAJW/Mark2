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
	animated(const std::shared_ptr<const resource::image>& image);
	void tick(double dt);
	auto render(
		vector<double> pos,
		float size,
		float rotation,
		const sf::Color& color) const -> sprite;

private:
	std::shared_ptr<const resource::image> m_image;
	float m_state = 0.f;
};
} // namespace model
} // namespace mark