﻿#include "stdafx.h"

#include "renderer.h"
#include "resource_image.h"
#include "sprite.h"
#include "update_context.h"

namespace mark {

// Render sprite using world coordinates
static void render(
	sf::RenderTexture& buffer,
	const sprite& sprite,
	const vd& camera,
	const vd& resolution)
{
	Expects(sprite.image);
	sf::Sprite tmp;
	let texture_size = static_cast<float>(sprite.image->size().y);
	let scale = sprite.size / texture_size;
	tmp.setTexture(sprite.image->texture());
	if (sprite.frame != sprite::all) {
		tmp.setTextureRect(
			{ static_cast<int>(texture_size) * gsl::narrow<int>(sprite.frame),
			  0,
			  static_cast<int>(texture_size),
			  static_cast<int>(texture_size) });
		tmp.scale(scale, scale);
	}
	if (sprite.centred) {
		tmp.setOrigin(texture_size / 2.f, texture_size / 2.f);
	}
	tmp.rotate(sprite.rotation);
	tmp.setColor(sprite.color);
	let offset = [&] {
		if (let screen_pos = std::get_if<vi32>(&sprite.pos)) {
			return vector<float>(*screen_pos);
		}
		return vector<float>(std::get<vd>(sprite.pos) - camera + resolution / 2.);
	}();
	tmp.move(vector<float>(offset));
	buffer.draw(tmp);
}

static void
render(sf::RenderTexture& buffer, const path& path, const vd& camera)
{
	std::vector<sf::Vertex> points;
	if (path.points.size() <= 1) {
		return;
	}
	if (path.world) {
		for (size_t i = 0; i < path.points.size() - 1; ++i) {
			let cur = path.points[i];
			let next = path.points[i + 1];
			points.push_back(sf::Vertex(vector<float>(cur - camera)));
			points.push_back(sf::Vertex(vector<float>(next - camera)));
		}
	} else {
		for (size_t i = 0; i < path.points.size() - 1; ++i) {
			let cur = path.points[i];
			let next = path.points[i + 1];
			points.push_back(sf::Vertex(vector<float>(cur)));
			points.push_back(sf::Vertex(vector<float>(next)));
		}
	}
	buffer.draw(points.data(), points.size(), sf::Lines);
}

static void render(sf::RenderTexture& buffer, const rectangle& rect)
{
	sf::RectangleShape rectangle;
	rectangle.setPosition(sf::Vector2f(rect.pos));
	rectangle.setSize(sf::Vector2f(rect.size));
	rectangle.setFillColor(rect.background_color);
	rectangle.setOutlineColor(rect.border_color);
	buffer.draw(rectangle);
}

static void render(
	sf::RenderTexture& buffer,
	const std::variant<sprite, path, rectangle>& any,
	const vd& camera,
	const vd& resolution)
{
	if (let sprite = std::get_if<mark::sprite>(&any)) {
		render(buffer, *sprite, camera, resolution);
	} else if (let path = std::get_if<mark::path>(&any)) {
		render(buffer, *path, camera);
	} else if (let rect = std::get_if<rectangle>(&any)) {
		render(buffer, *rect);
	}
}

} // namespace mark

mark::renderer::renderer(vu32 res)
{
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

sf::Sprite mark::renderer::render(const render_info& info)
{
	let camera = info.camera;
	let resolution = info.resolution;
	let shadow_res = m_vbo->getSize().x;

	if (vd(m_buffer->getSize()) != resolution) {
		let res = vu32(resolution);
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
	m_shadows_shader.setUniform(
		"shadow_resolution", static_cast<float>(shadow_res));
	m_shadows_shader.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_bump_mapping.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_occlusion_shader.setUniform("shadow_res", static_cast<float>(shadow_res));
	m_buffer->clear({ 0, 0, 0, 0 });
	m_buffer2->clear();
	m_ui_layer->clear({ 0, 0, 0, 0 });
	m_vbo->clear(sf::Color::White);
	m_occlusion_map->clear();
	m_normal_map->clear({ 0x7E, 0x7E, 0xFF, 0xFF }); // normal flat surface

	std::vector<sf::Glsl::Vec2> lights_pos;
	std::vector<sf::Glsl::Vec4> lights_color;

	for (let& pair : info.lights) {
		let pos = pair.first - camera;
		let color = pair.second;
		if (pos.x >= -resolution.x / 2.0 - 160.0
			&& pos.x <= resolution.x / 2.0 + 160.0
			&& pos.y >= -resolution.y / 2.0 - 160.0
			&& pos.y <= resolution.y / 2.0 + 160.0) {
			lights_color.push_back(color);
			lights_pos.push_back(vector<float>(pos));
		}
	}
	let lights_count = std::min(lights_pos.size(), static_cast<size_t>(64));

	for (let& layer : info.sprites) {
		if (layer.first < 0) {
			for (let& sprites : layer.second) {
				mark::render(*m_occlusion_map, sprites, camera, resolution);
			}
		} else if (layer.first < 100) {
			for (let& sprite : layer.second) {
				mark::render(*m_buffer, sprite, camera, resolution);
			}
		} else {
			for (let& sprite : layer.second) {
				mark::render(*m_ui_layer, sprite, camera, resolution);
			}
		}
	}
	for (let& layer : info.normals) {
		for (let& sprite : layer.second) {
			mark::render(*m_normal_map, sprite, camera, resolution);
		}
	}

	m_occlusion_map->display();
	m_normal_map->display();
	m_ui_layer->display();
	sf::Sprite sprite1(m_occlusion_map->getTexture());
	sprite1.scale(
		{ static_cast<float>(shadow_res) / static_cast<float>(resolution.x),
		  1.f / static_cast<float>(resolution.y) });
	m_vbo->draw(sprite1, &m_occlusion_shader);
	m_vbo->display();
	m_buffer->display();
	m_buffer2->draw(sf::Sprite(m_occlusion_map->getTexture()));
	m_buffer2->draw(sf::Sprite(m_normal_map->getTexture()), &m_bump_mapping);
	m_buffer2->draw(sf::Sprite(m_buffer->getTexture()));
	m_buffer2->draw(sf::Sprite(m_vbo->getTexture()));
	sf::Sprite shadows(m_vbo->getTexture());
	shadows.setScale(
		{ static_cast<float>(resolution.x) / static_cast<float>(shadow_res),
		  static_cast<float>(resolution.y / 1.f) });
	if (!lights_pos.empty()) {
		m_shadows_shader.setUniformArray(
			"lights_pos", lights_pos.data(), lights_count);
		m_shadows_shader.setUniformArray(
			"lights_color", lights_color.data(), lights_count);
	}
	m_shadows_shader.setUniform("lights_count", static_cast<int>(lights_count));
	m_buffer2->draw(shadows, &m_shadows_shader);
	m_buffer2->draw(sf::Sprite(m_ui_layer->getTexture()));
	m_buffer2->display();
	return sf::Sprite(m_buffer2->getTexture());
}
