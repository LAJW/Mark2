#pragma once
#include <SFML/Graphics.hpp>
#include "vector.h"

namespace mark {
	struct tick_context;

	class renderer {
	public:
		renderer();
		void render(
			const mark::tick_context& context,
			mark::vector<double> camera,
			mark::vector<double> resolution,
			sf::RenderWindow& window);
	private:
		sf::RenderTexture m_buffer;
		sf::RenderTexture m_buffer2;
		sf::RenderTexture m_occlusion_map;
		sf::RenderTexture m_normal_map;
		sf::RenderTexture m_ui_layer;
		sf::RenderTexture m_vbo;
		sf::Shader m_occlusion_shader;
		sf::Shader m_shadows_shader;
		sf::Shader m_bump_mapping;
	};
}