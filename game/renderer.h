#pragma once
#include "stdafx.h"
#include <SFML/Graphics.hpp>

namespace mark {
	struct tick_context;
	class sprite;

	class renderer {
	public:
		renderer(mark::vector<unsigned> res, unsigned shadow_res);

		struct render_info {
			mark::vector<double> camera;
			mark::vector<double> resolution;
			std::vector<std::pair<mark::vector<double>, sf::Color>> lights;
			std::map<int, std::vector<mark::sprite>> sprites;
			std::map<int, std::vector<mark::sprite>> ui_sprites;
			std::map<int, std::vector<mark::sprite>> normals;
		};
		sf::Sprite render(const render_info& info);
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