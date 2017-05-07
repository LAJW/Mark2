#include <algorithm>
#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"
#include "world.h"
#include "resource_manager.h"
#include "unit_bucket.h"
#include <sstream>

mark::module::cargo::cargo(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }, resource_manager.image("cargo.png")),
	m_im_body(resource_manager.image("cargo.png")),
	m_im_light(resource_manager.image("glare.png")),
	m_lfo(0.5f, resource_manager.random(0.f, 6.f)),
	m_modules(64) {
}

void mark::module::cargo::tick(mark::tick_context& context) {
	m_lfo.tick(context.dt);
	auto pos = this->pos();
	auto light_offset = mark::rotate(mark::vector<double>(24.f, 8.f), parent().rotation());
	auto light_strength = static_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	{
		mark::sprite::arguments info;
		info.image = m_im_body;
		info.pos = pos;
		info.size = 64.f;
		info.rotation = parent().rotation();
		context.sprites[0].emplace_back(info);
	}
	{
		mark::sprite::arguments info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 32.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[1].emplace_back(info);
	}
	{
		mark::sprite::arguments info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 16.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[2].emplace_back(info);
	}
	mark::tick_context::bar_info bar;
	bar.image = parent().world().resource_manager().image("bar.png");
	bar.pos = pos + mark::vector<double>(0, -mark::module::size * 2.0);
	bar.type = mark::tick_context::bar_type::health;
	bar.percentage = m_cur_health / m_max_health;
	context.render(bar);
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
	mark::vector<int> i_pos,
	const std::unique_ptr<mark::module::base>& module) const -> bool {
	if (i_pos.x < 0 || i_pos.y < 0) {
		return false;
	}
	const auto pos = mark::vector<unsigned>(i_pos);
	for (unsigned i = 0; i < m_modules.size(); i++) {
		const mark::vector<unsigned> module_pos(i % 16, i / 16);
		auto& slot = m_modules[i];
		if (slot) {
			const auto border = module_pos + mark::vector<unsigned>(slot->size());
			if (pos.x + module->size().x >= module_pos.x && pos.x < border.x
				&& pos.y + module->size().y >= module_pos.y && pos.y < border.y) {
				return false;
			}
		}
	}
	return true;
}

auto mark::module::cargo::module(mark::vector<int> pos) -> mark::module::base*{
	return const_cast<mark::module::base*>(static_cast<const mark::module::cargo*>(this)->module(pos));
}

auto mark::module::cargo::module(mark::vector<int> i_pos) const -> const mark::module::base* {
	if (i_pos.x < 0 || i_pos.y < 0) {
		return false;
	}
	const auto pos = mark::vector<unsigned>(i_pos);
	for (unsigned i = 0; i < m_modules.size(); i++) {
		const mark::vector<unsigned> module_pos(i % 16, i / 16);
		auto& slot = m_modules[i];
		if (slot) {
			const auto border = module_pos + mark::vector<unsigned>(slot->size());
			if (pos.x + pos.x >= module_pos.x && pos.x < border.x
				&& pos.y + pos.y >= module_pos.y && pos.y < border.y) {
				return slot.get();
			}
		}
	}
	return nullptr;
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
	const auto image = parent().world().resource_manager().image("grid-background.png");
	auto size = this->interior_size();
	for (const auto point : mark::area(size)) {
		const auto slot_pos = pos_in + mark::vector<double>(point * 16);
		mark::sprite::arguments info;
		info.image = image;
		info.pos = slot_pos;
		context.sprites[0].emplace_back(info);
		const auto& module = m_modules[point.x + point.y * 16].get();
		if (module) {
			const auto module_pos = slot_pos + mark::vector<double>(module->size()) * 16.0 / 2.0 - mark::vector<double>(8, 8);
			const auto size = static_cast<float>(std::max(module->size().x, module->size().y)) * 16.f;
			mark::sprite::arguments info;
			info.image = module->thumbnail();
			info.pos = module_pos;
			info.size = size;
			context.sprites[1].emplace_back(info);
		}
	}
}

auto mark::module::cargo::interior_size() const -> mark::vector<int> {
	auto size_v = static_cast<int>(m_modules.size());
	return mark::vector<int>(16, size_v / 16);
}

auto mark::module::cargo::detachable() const -> bool {
	for (const auto& module : m_modules) {
		if (module) {
			return false;
		}
	}
	return true;
}

std::string mark::module::cargo::describe() const {
	std::ostringstream os;
	os << "Cargo Module" << std::endl;
	os << "Capacity: " << m_modules.size() << std::endl;
	return os.str();
}

void mark::module::cargo::on_death(mark::tick_context & context) {
	mark::module::base::on_death(context);
	for (auto& module : m_modules) {
		if (module) {
			context.units.push_back(std::make_shared<mark::unit::bucket>(parent().world(), pos(), std::move(module)));
		}
	}
}

bool mark::module::cargo::push(std::unique_ptr<mark::module::base>& module) {
	for (unsigned i = 0; i < m_modules.size(); i++) {
		mark::vector<int> drop_pos(i % 16, i / 16);
		if (this->can_drop(drop_pos, module)) {
			this->drop(drop_pos, std::move(module));
			return true;
		}
	}
	return false;
}
