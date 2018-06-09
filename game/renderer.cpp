#include "stdafx.h"

#include "renderer.h"
#include "resource_image.h"
#include "sprite.h"
#include "update_context.h"

namespace mark {

const constexpr auto shadow_resolution = 512u;
const constexpr auto shadow_resolutionf = static_cast<float>(shadow_resolution);

namespace {
struct render_info final
{
	optional<renderer::window_buffers&> window_buffers;
	vd camera;
	vd resolution;
};
} // namespace

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
		return vector<float>(
			std::get<vd>(sprite.pos) - camera + resolution / 2.);
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
	const mark::render_info& info,
	sf::RenderTexture& buffer,
	const mark::window& window)
{
	// TODO: Rotate buffers
	auto& window_buffer = *(*info.window_buffers)[0];
	for (let& [ z_index, sprites] : window.sprites) {
		for (let& renderable : sprites) {
			if (let sprite = std::get_if<mark::sprite>(&renderable)) {
				render(window_buffer, *sprite, info.camera, info.resolution);
			} else if (let path = std::get_if<mark::path>(&renderable)) {
				render(window_buffer, *path, info.camera);
			} else if (let rect = std::get_if<rectangle>(&renderable)) {
				render(window_buffer, *rect);
			}
		}
	}
	buffer.draw([&] {
		sf::Sprite sprite(window_buffer.getTexture());
		// sprite.move(vector<float>(window.pos));
		// sprite.setTextureRect({ window.scroll, window.size });
		return sprite;
	}());
}

static void render(
	const render_info& info,
	sf::RenderTexture& buffer,
	const renderable& any)
{
	if (let sprite = std::get_if<mark::sprite>(&any)) {
		return render(buffer, *sprite, info.camera, info.resolution);
	}
	if (let path = std::get_if<mark::path>(&any)) {
		return render(buffer, *path, info.camera);
	}
	if (let rect = std::get_if<rectangle>(&any)) {
		return render(buffer, *rect);
	}
	if (let window = std::get_if<mark::window>(&any)) {
		return render(info, buffer, *window);
	}
	Expects(false); // Unreachable
}

static unique_ptr<sf::RenderTexture>
make_render_texture(const vu32& resolution, bool smooth = false)
{
	auto buffer = std::make_unique<sf::RenderTexture>();
	buffer->create(resolution.x, resolution.y);
	buffer->setSmooth(smooth);
	return buffer;
}

void mark::renderer::resolution(const vu32& resolution)
{
	if (m_buffer && m_buffer->getSize() == resolution) {
		return;
	}
	m_buffer = make_render_texture(resolution);
	m_buffer2 = make_render_texture(resolution);
	m_occlusion_map = make_render_texture(resolution);
	m_normal_map = make_render_texture(resolution);
	m_ui_layer = make_render_texture(resolution);
	for (auto& buffer : m_window_buffers) {
		buffer = make_render_texture(resolution);
	}
}

mark::renderer::renderer(const vu32& resolution)
	: m_vbo(make_render_texture({ shadow_resolution, 1 }, true))
{
	this->resolution(resolution);
	m_occlusion_shader.loadFromFile("occlusion.glsl", sf::Shader::Fragment);
	m_shadows_shader.loadFromFile("shadows.glsl", sf::Shader::Fragment);
	m_bump_mapping.loadFromFile("bump_mapping.glsl", sf::Shader::Fragment);
}

void mark::renderer::clear()
{
	// Clear with white to highlight opaque objects (shields, projectiles)
	// Side effect of this is making shadows brighter
	m_buffer->clear({ 255, 255, 255, 0 });
	m_buffer2->clear();
	m_ui_layer->clear({ 0, 0, 0, 0 });
	m_vbo->clear(sf::Color::White);
	m_occlusion_map->clear();
	m_normal_map->clear({ 0x7E, 0x7E, 0xFF, 0xFF }); // normal flat surface
	for (auto& buffer : m_window_buffers) {
		buffer->clear({ 0, 0, 0, 0 });
	}
}

sf::Sprite mark::renderer::render(const render_info& info)
{
	let camera = info.camera;
	let resolution = info.resolution;
	this->resolution(vu32(resolution));
	this->clear();
	m_shadows_shader.setUniform("shadow_resolution", shadow_resolutionf);
	m_shadows_shader.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_bump_mapping.setUniform("resolution", sf::Glsl::Vec2(resolution));
	m_occlusion_shader.setUniform("shadow_res", shadow_resolutionf);

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

	mark::render_info render_info;
	render_info.camera = camera;
	render_info.resolution = resolution;
	render_info.window_buffers = m_window_buffers;
	for (let& layer : info.sprites) {
		if (layer.first < 0) {
			for (let& sprites : layer.second) {
				mark::render(render_info, *m_occlusion_map, sprites);
			}
		} else if (layer.first < 100) {
			for (let& sprite : layer.second) {
				mark::render(render_info, *m_buffer, sprite);
			}
		} else {
			for (let& sprite : layer.second) {
				mark::render(render_info, *m_ui_layer, sprite);
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
	sprite1.scale({ shadow_resolutionf / static_cast<float>(resolution.x),
					1.f / static_cast<float>(resolution.y) });
	m_vbo->draw(sprite1, &m_occlusion_shader);
	m_vbo->display();
	m_buffer->display();
	m_buffer2->draw(sf::Sprite(m_occlusion_map->getTexture()));
	m_buffer2->draw(sf::Sprite(m_normal_map->getTexture()), &m_bump_mapping);
	m_buffer2->draw(sf::Sprite(m_buffer->getTexture()));
	m_buffer2->draw(sf::Sprite(m_vbo->getTexture()));
	sf::Sprite shadows(m_vbo->getTexture());
	shadows.setScale({ static_cast<float>(resolution.x) / shadow_resolutionf,
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

} // namespace mark
