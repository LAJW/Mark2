#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"
#include "world.h"
#include "resource_manager.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }, resource_manager.image("cargo.png")),
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

void mark::module::cargo::drop(mark::vector<int> pos, std::unique_ptr<mark::module::base> module) {
	try {
		const auto vector_pos = pos.y * 16 + pos.x;
		auto& place = m_modules.at(vector_pos);
		if (place == nullptr) {
			place = std::move(module);
		} else {
			throw mark::exception("OCCUPIED");
		}
	} catch (std::runtime_error&) {
		throw mark::exception("BAD_POS");
	}
	if (pos.x < 0 && pos.y < 0) {
		throw mark::exception("BAS_POS");
	}
}

auto mark::module::cargo::can_drop(
	mark::vector<int> pos,
	const std::unique_ptr<mark::module::base>& module) const -> bool {
	if (pos.x < 0 || pos.y < 0) {
		return false;
	}
	for (int i = 0; i < m_modules.size(); i++) {
		const mark::vector<int> module_pos(i % 16, i / 16);
		auto& slot = m_modules[i];
		if (slot) {
			const auto border = module_pos + mark::vector<int>(slot->size());
			if (pos.x + module->size().x >= module_pos.x && pos.x < border.x
				&& pos.y + module->size().y >= module_pos.y && pos.y < border.y) {
				return false;
			}
		}
	}
	return true;
}

auto mark::module::cargo::pick(mark::vector<int> pos) -> std::unique_ptr<mark::module::base> {
	if (pos.x < 0 && pos.y < 0) {
		return nullptr;
	}
	for (int i = 0; i < m_modules.size(); i++) {
		const mark::vector<int> module_pos(i % 16, i / 16);
		auto& module = m_modules[i];
		if (module) {
			const auto border = module_pos + mark::vector<int>(module->size());
			if (pos.x >= module_pos.x && pos.x < border.x
				&& pos.y >= module_pos.y && pos.y < border.y) {
				return std::move(module);
			}
		}
	}
	return nullptr;
}

void mark::module::cargo::render_contents(mark::vector<double> pos_in, mark::tick_context & context) {
	const auto image = socket()->world().resource_manager().image("grid-background.png");
	auto size = this->interior_size();
	for (const auto point : mark::area(size)) {
		const auto slot_pos = pos_in + mark::vector<double>(point * 16);
		context.sprites[0].push_back(mark::sprite(image, slot_pos));
		const auto& module = m_modules[point.x + point.y * 16].get();
		if (module) {
			const auto module_pos = slot_pos + mark::vector<double>(module->size()) * 16.0 / 2.0 - mark::vector<double>(8, 8);
			const auto size = static_cast<double>(std::max(module->size().x, module->size().y)) * 16.f;
			context.sprites[1].push_back(mark::sprite(module->thumbnail(), module_pos, size));
		}
	}
}

auto mark::module::cargo::interior_size() const -> mark::vector<int> {
	auto size_v = m_modules.size();
	return mark::vector<int>(16, size_v / 16);
}
