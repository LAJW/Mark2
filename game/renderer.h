#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "vector.h"

namespace mark {
	struct tick_context;

	class renderer {
	public:
		renderer(mark::vector<unsigned> res, unsigned shadow_res);
		void render(
			const mark::tick_context& context,
			mark::vector<double> camera,
			mark::vector<double> resolution,
			sf::RenderWindow& window);
	private:
		std::unique_ptr<sf::RenderTexture> m_buffer;
		std::unique_ptr<sf::RenderTexture> m_buffer2;
		std::unique_ptr<sf::RenderTexture> m_occlusion_map;
		std::unique_ptr<sf::RenderTexture> m_normal_map;
		std::unique_ptr<sf::RenderTexture> m_ui_layer;
		std::unique_ptr<sf::RenderTexture> m_vbo;
		sf::Shader m_occlusion_shader;
		sf::Shader m_shadows_shader;
		sf::Shader m_bump_mapping;
	};
}