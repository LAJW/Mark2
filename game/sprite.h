#pragma once
#include <memory>
#include "vector.h"
#include <SFML/Graphics/Color.hpp>

namespace mark {
	namespace resource {
		class image;
	}
	class sprite {
	public:
		struct info {
			std::shared_ptr<const mark::resource::image> image;
			mark::vector<double> pos;
			float size = 16.f;
			float rotation = 0.f;
			size_t frame = 0;
			sf::Color color = sf::Color::White;
		};
		sprite(mark::sprite::info& args);
		auto image() const -> const mark::resource::image&;
		auto x() const -> double;
		auto y() const -> double;
		auto rotation() const -> float;
		auto size() const -> float;
		auto frame() const -> size_t { return m_frame; }
		inline auto color() const -> sf::Color { return m_color; }
	private:
		std::shared_ptr<const mark::resource::image> m_image;
		double m_x;
		double m_y;
		float m_size;
		float m_rotation;
		size_t m_frame;
		sf::Color m_color;
	};
}