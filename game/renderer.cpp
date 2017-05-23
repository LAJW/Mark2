#include "renderer.h"
#include "tick_context.h"

namespace {
	void static render(const mark::sprite& sprite, const mark::vector<double>& camera, sf::RenderTexture& buffer, mark::vector<double> resolution) {
		sf::Sprite tmp;
		const auto texture_size = static_cast<float>(sprite.image().getSize().y);
		const auto scale = sprite.size() / texture_size;
		tmp.setTexture(sprite.image());
		tmp.setTextureRect({ static_cast<int>(texture_size) * static_cast<int>(sprite.frame()), 0, static_cast<int>(texture_size), static_cast<int>(texture_size)  });
		tmp.setOrigin(texture_size / 2.f, texture_size / 2.f);
		tmp.scale(scale, scale);
		tmp.rotate(sprite.rotation());
		tmp.setColor(sprite.color());
		tmp.move(static_cast<float>(sprite.x() - camera.x + resolution.x / 2.0), static_cast<float>(sprite.y() - camera.y + resolution.y / 2.0));
		buffer.draw(tmp);
	}
}

mark::renderer::renderer(mark::vector<unsigned> res, unsigned shadow_res) {
	m_buffer = std::make_unique<sf::RenderTexture>();
	m_buffer->create(res.x, res.y);
	m_buffer2 = std::make_unique<sf::RenderTexture>();
	m_buffer2->create(res.x, res.y);
	m_occlusion_map = std::make_unique<sf::RenderTexture>();
	m_occlusion_map->create(res.x, res.y);
	m_normal_map = std::make_unique<sf::RenderTexture>();
	m_normal_map->create(res.x, res.y);
	m_ui_layer = std::make_unique<sf::RenderTexture>();
	m_ui_layer->create(res.x, res.y);
	m_vbo = std::make_unique<sf::RenderTexture>();
	m_vbo->create(512, 1);
	m_vbo->setSmooth(true);
	m_occlusion_shader.loadFromFile("occlusion.glsl", sf::Shader::Fragment);
	m_shadows_shader.loadFromFile("shadows.glsl", sf::Shader::Fragment);
	m_bump_mapping.loadFromFile("bump_mapping.glsl", sf::Shader::Fragment);
}

void mark::renderer::render(
	const mark::tick_context& context,
	mark::vector<double> camera,
	mark::vector<double> resolution,
	sf::RenderWindow& window) {

	const auto shadow_res = m_vbo->getSize().x;

	if (mark::vector<double>(m_buffer->getSize()) != resolution) {
		const auto res = mark::vector<unsigned>(resolution);
		m_buffer = std::make_unique<sf::RenderTexture>();
		m_buffer->create(res.x, res.y);
		m_buffer2 = std::make_unique<sf::RenderTexture>();
		m_buffer2->create(res.x, res.y);
		m_occlusion_map = std::make_unique<sf::RenderTexture>();
		m_occlusion_map->create(res.x, res.y);
		m_normal_map = std::make_unique<sf::RenderTexture>();
		m_normal_map->create(res.x, res.y);
		m_ui_layer = std::make_unique<sf::RenderTexture>();
		m_ui_layer->create(res.x, res.y);
	}
	m_shadows_shader.setUniform("shadow_resolution", static_cast<float>(shadow_res));
	m_shadows_shader.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_bump_mapping.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_occlusion_shader.setUniform("shadow_res", static_cast<float>(shadow_res));
	m_buffer->clear({ 0, 0, 0, 0 });
	m_buffer2->clear();
	m_ui_layer->clear({ 0, 0, 0, 0 });
	m_vbo->clear(sf::Color::White);
	m_occlusion_map->clear();
	m_normal_map->clear({ 0x7E, 0x7E, 0xFF, 0xFF }); // normal flat surface

	const auto& sprites = context.sprites;
	const auto& normals = context.normals;

	std::vector<sf::Glsl::Vec2> lights_pos;
	std::vector<sf::Glsl::Vec4> lights_color;

	for (const auto& pair : context.lights) {
		const auto pos = pair.first - camera;
		const auto color = pair.second;
		if (pos.x >= -resolution.x / 2.0 - 160.0 && pos.x <= resolution.x / 2.0 + 160.0
			&& pos.y >= -resolution.y / 2.0 - 160.0 && pos.y <= resolution.y / 2.0 + 160.0) {
			lights_color.push_back(color);
			lights_pos.push_back(mark::vector<float>(pos));
		}
	}
	const auto lights_count = std::min(
		lights_pos.size(),
		static_cast<size_t>(64)
	);

	for (const auto& layer : sprites) {
		if (layer.first < 0) {
			for (const auto& sprite : layer.second) {
				::render(sprite, camera, *m_occlusion_map, resolution);
			}
		} else if (layer.first < 100) {
			for (const auto& sprite : layer.second) {
				::render(sprite, camera, *m_buffer, resolution);
			}
		} else {
			for (const auto& sprite : layer.second) {
				::render(sprite, camera, *m_ui_layer, resolution);
			}
		}
	}

	for (const auto& layer : normals) {
		for (const auto& sprite : layer.second) {
			::render(sprite, camera, *m_normal_map, resolution);
		}
	}

	m_occlusion_map->display();
	m_normal_map->display();
	m_ui_layer->display();
	sf::Sprite sprite1(m_occlusion_map->getTexture());
	sprite1.scale({ static_cast<float>(shadow_res) / static_cast<float>(resolution.x), 1.f / static_cast<float>(resolution.y) });
	m_vbo->draw(sprite1, &m_occlusion_shader);
	m_vbo->display();
	m_buffer->display();
	m_buffer2->draw(sf::Sprite(m_occlusion_map->getTexture()));
	m_buffer2->draw(sf::Sprite(m_normal_map->getTexture()), &m_bump_mapping);
	m_buffer2->draw(sf::Sprite(m_buffer->getTexture()));
	m_buffer2->draw(sf::Sprite(m_vbo->getTexture()));
	sf::Sprite shadows(m_vbo->getTexture());
	shadows.setScale({ static_cast<float>(resolution.x) / static_cast<float>(shadow_res), static_cast<float>(resolution.y / 1.f) });;
	// m_shadows_shader.setUniformArray("lights_pos", lights_pos.data(), lights_count);
	m_shadows_shader.setUniformArray("lights_color", lights_color.data(), lights_count);
	m_shadows_shader.setUniform("lights_count", static_cast<int>(lights_count));
	m_buffer2->draw(shadows, &m_shadows_shader);
	m_buffer2->draw(sf::Sprite(m_ui_layer->getTexture()));
	m_buffer2->display();
	window.draw(sf::Sprite(m_buffer2->getTexture()));
	window.display();
}