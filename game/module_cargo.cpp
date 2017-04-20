#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }),
	m_im_body(resource_manager.image("cargo.png")),
	m_im_light(resource_manager.image("glare.png")),
	m_lfo(0.5f, static_cast<float>(resource_manager.random_double(0, 6))),
	m_modules(64) {
}

void mark::module::cargo::tick(mark::tick_context& context) {
	m_lfo.tick(context.dt);
	auto pos = socket()->relative_pos();
	auto light_offset = mark::rotate(mark::vector<double>(24.f, 8.f), socket()->rotation());
	auto light_strength = static_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	context.sprites[0].push_back(mark::sprite(m_im_body, pos, 64.f, socket()->rotation()));
	context.sprites[1].push_back(mark::sprite(m_im_light, pos + light_offset, 32.f, 0, 0, sf::Color(255, 200, 150, light_strength)));
	context.sprites[2].push_back(mark::sprite(m_im_light, pos + light_offset, 16.f, 0, 0, sf::Color(255, 255, 255, light_strength)));
}

auto mark::module::cargo::modules() -> std::vector<std::unique_ptr<mark::module::base>>& {
	return m_modules;
}

auto mark::module::cargo::drop(mark::vector<int> pos, std::unique_ptr<mark::module::base> module) {
	try {
		auto& place = m_modules.at(pos.y * 8 + pos.x);
		if (place == nullptr) {
			place = std::move(module);
		} else {
			throw mark::exception("OCCUPIED");
		}
	} catch (std::runtime_error&) {
		throw mark::exception("BAD_POS");
	}
}

auto mark::module::cargo::pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base> {
	try {
		return std::move(m_modules.at(pos.y * 8 + pos.x));
	} catch (std::runtime_error&) {
		return nullptr;
	}
}
