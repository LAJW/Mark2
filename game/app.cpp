#include <chrono>
#include "app.h"
#include <SFML/Graphics.hpp>
#include <yaml-cpp/yaml.h>
#include "resource_image.h"
#include "resource_manager.h"
#include "world.h"
#include "sprite.h"
#include "vector.h"
#include <iostream>
#include "command.h"
#include "tick_context.h"
#include "keymap.h"

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
}

void mark::app::main() {
	const auto keymap = mark::keymap("options.yml");
	auto world = std::make_unique<mark::world>(m_resource_manager);

	auto last = std::chrono::system_clock::now();

	sf::RenderTexture buffer;
	buffer.create(1920, 1080);
	sf::RenderTexture buffer2;
	buffer2.create(1920, 1080);
	sf::RenderTexture occlusion_map;
	occlusion_map.create(1920, 1080);
	sf::RenderTexture normal_map;
	normal_map.create(1920, 1080);
	sf::RenderTexture ui_layer;
	ui_layer.create(1920, 1080);
	sf::RenderTexture vbo;
	vbo.create(512, 1);
	vbo.setSmooth(true);
	sf::Shader occlusion_shader;
	occlusion_shader.loadFromFile("occlusion.glsl", sf::Shader::Fragment);
	sf::Shader shadows_shader;
	shadows_shader.loadFromFile("shadows.glsl", sf::Shader::Fragment);
	sf::Shader bump_mapping;
	bump_mapping.loadFromFile("bump_mapping.glsl", sf::Shader::Fragment);

	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;

		if (dt >= 1.0 / 60.0) {
			last = now;
			sf::Event event;

			const auto target = world->camera() + mark::vector<double>(sf::Mouse::getPosition(m_window)) - mark::vector<double>(m_window.getSize()) / 2.0;
			while (m_window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					m_window.close();
				} else {
					auto command = keymap.translate(event);
					command.pos = target;
					if (command.type == mark::command::type::reset) {
						world = std::make_unique<mark::world>(m_resource_manager);
					} else {
						world->command(command);
					}
				}
			}
			mark::command guide;
			guide.type = mark::command::type::guide;
			guide.pos = target;
			world->command(guide);

			m_window.clear();
			buffer.clear({ 0, 0, 0, 0 });
			buffer2.clear();
			ui_layer.clear({ 0, 0, 0, 0 });
			vbo.clear(sf::Color::White);
			occlusion_map.clear();
			normal_map.clear({ 0x7E, 0x7E, 0xFF, 0xFF });
			mark::tick_context context(m_resource_manager);
			context.dt = dt;
			const auto resolution = mark::vector<double>(m_window.getSize());
			world->tick(context, resolution);
			const auto& sprites = context.sprites;
			const auto& normals = context.normals;

			std::vector<sf::Glsl::Vec2> lights_pos;
			std::vector<sf::Glsl::Vec4> lights_color;

			for (const auto& pair : context.lights) {
				const auto pos = pair.first - world->camera();
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
						render(sprite, world->camera(), occlusion_map);
					}
				} else if (layer.first < 100) {
					for (const auto& sprite : layer.second) {
						render(sprite, world->camera(), buffer);
					}
				} else {
					for (const auto& sprite : layer.second) {
						render(sprite, world->camera(), ui_layer);
					}
				}
			}

			for (const auto& layer : normals) {
				for (const auto& sprite : layer.second) {
					render(sprite, world->camera(), normal_map);
				}
			}

			occlusion_map.display();
			normal_map.display();
			ui_layer.display();
			sf::Sprite sprite1(occlusion_map.getTexture());
			sprite1.scale({ 512.f / 1920.f, 1.f / 1080.f });
			vbo.draw(sprite1, &occlusion_shader);
			vbo.display();
			buffer.display();
			buffer2.draw(sf::Sprite(occlusion_map.getTexture()));
			buffer2.draw(sf::Sprite(normal_map.getTexture()), &bump_mapping);
			buffer2.draw(sf::Sprite(buffer.getTexture()));
			buffer2.draw(sf::Sprite(vbo.getTexture()));
			sf::Sprite shadows(vbo.getTexture());
			shadows.setScale({ 1920.f / 512.f, 1080.f / 1.f });;
			shadows_shader.setUniformArray("lights_pos", lights_pos.data(), lights_count);
			shadows_shader.setUniformArray("lights_color", lights_color.data(), lights_count);
			shadows_shader.setUniform("lights_count", static_cast<int>(lights_count));
			buffer2.draw(shadows, &shadows_shader);
			buffer2.draw(sf::Sprite(ui_layer.getTexture()));
			buffer2.display();
			m_window.draw(sf::Sprite(buffer2.getTexture()));
			m_window.display();
		}
	}
}

void mark::app::render(const mark::sprite& sprite, const mark::vector<double>& camera, sf::RenderTexture& buffer) {
	sf::Sprite tmp;
	const auto texture_size = static_cast<float>(sprite.image().getSize().y);
	const auto scale = sprite.size() / texture_size;
	tmp.setTexture(sprite.image());
	tmp.setTextureRect({ static_cast<int>(texture_size) * static_cast<int>(sprite.frame()), 0, static_cast<int>(texture_size), static_cast<int>(texture_size)  });
	tmp.setOrigin(texture_size / 2.f, texture_size / 2.f);
	tmp.scale(scale, scale);
	tmp.rotate(sprite.rotation());
	tmp.setColor(sprite.color());
	tmp.move(static_cast<float>(sprite.x() - camera.x + m_window.getSize().x / 2.0), static_cast<float>(sprite.y() - camera.y + m_window.getSize().y / 2.0));
	buffer.draw(tmp);
}