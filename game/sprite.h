#pragma once
#include <memory>
#include "resource_image.h"

namespace mark {
	class sprite {
	public:
		sprite(std::shared_ptr<const mark::resource::image> image, double x, double y, float size = 16.f, float rotation = 0.f, size_t frame = 0);
		auto image() const -> const mark::resource::image&;
		auto x() const -> double;
		auto y() const -> double;
		auto rotation() const -> float;
		auto size() const -> float;
	private:
		std::shared_ptr<const mark::resource::image> m_image;
		double m_x;
		double m_y;
		float m_size;
		float m_rotation;
		size_t m_frame;
	};
}