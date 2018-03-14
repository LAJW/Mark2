#pragma once
#include "stdafx.h"
#include <SFML/Graphics.hpp>

namespace mark {
struct tick_context;
struct sprite;

class renderer {
public:
	renderer(vector<unsigned> res);

	struct render_info {
		vector<double> camera;
		vector<double> resolution;
		std::vector<std::pair<vector<double>, sf::Color>> lights;
		std::map<int, std::vector<std::variant<sprite, path, rectangle>>>
			sprites;
		std::map<int, std::vector<sprite>> normals;
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
} // namespace mark