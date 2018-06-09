#pragma once
#include "stdafx.h"
#include <SFML/Graphics.hpp>

namespace mark {
struct update_context;
struct sprite;

class renderer final
{
public:
	using window_buffers = std::array<unique_ptr<sf::RenderTexture>, 4>;

	renderer(const vu32& res);

	struct render_info
	{
		vd camera;
		vd resolution;
		std::vector<std::pair<vd, sf::Color>> lights;
		std::map<int, std::vector<renderable>> sprites;
		std::map<int, std::vector<sprite>> normals;
	};
	sf::Sprite render(const render_info& info);

private:
	/// Update resolution of the renderer
	void resolution(const vu32&);
	void clear();
	unique_ptr<sf::RenderTexture> m_buffer;
	unique_ptr<sf::RenderTexture> m_buffer2;
	unique_ptr<sf::RenderTexture> m_occlusion_map;
	unique_ptr<sf::RenderTexture> m_normal_map;
	unique_ptr<sf::RenderTexture> m_ui_layer;
	unique_ptr<sf::RenderTexture> m_vbo;
	window_buffers m_window_buffers;
	sf::Shader m_occlusion_shader;
	sf::Shader m_shadows_shader;
	sf::Shader m_bump_mapping;
};
} // namespace mark
