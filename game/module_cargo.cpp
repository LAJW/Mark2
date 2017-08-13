#include "stdafx.h"
#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "exception.h"
#include "world.h"
#include "resource_manager.h"
#include "unit_bucket.h"
#include <sstream>
#include "algorithm.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }, resource_manager.image("cargo.png")),
	m_im_body(resource_manager.image("cargo.png")),
	m_im_light(resource_manager.image("glare.png")),
	m_lfo(0.5f, resource_manager.random(0.f, 6.f)),
	m_modules(64) {
}

void mark::module::cargo::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	m_lfo.tick(context.dt);
	auto pos = this->pos();
	auto light_offset = mark::rotate(mark::vector<double>(24.f, 8.f), parent().rotation());
	auto light_strength = static_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	{
		mark::sprite::info info;
		info.image = m_im_body;
		info.pos = pos;
		info.size = 64.f;
		info.rotation = parent().rotation();
		info.color = this->heat_color();
		context.sprites[2].emplace_back(info);
	}
	{
		mark::sprite::info info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 32.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[4].emplace_back(info);
	}
	{
		mark::sprite::info info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 16.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[4].emplace_back(info);
	}
}

auto mark::module::cargo::modules() -> std::vector<std::unique_ptr<mark::module::base>>& {
	return m_modules;
}

namespace {
	auto overlaps(
		const std::pair<mark::vector<unsigned>, mark::vector<unsigned>>& left,
		const std::pair<mark::vector<unsigned>, mark::vector<unsigned>>& right) -> bool {
		return left.first.x < right.second.x
			&& left.second.x > right.first.x
			&& left.first.y < right.second.y
			&& left.second.y > right.first.y;
	}
}

mark::error::guard mark::module::cargo::drop(
	mark::vector<int> spos,
	std::unique_ptr<mark::module::base>& incoming) {

	// Check if fits inside the container
	const auto cargo_size = mark::vector<size_t>(16, m_modules.size() / 16);
	if (spos.x < 0 || spos.y < 0
		|| spos.x + incoming->size().x > cargo_size.x
		|| spos.y + incoming->size().y > cargo_size.y) {
		return mark::error::code::bad_pos;
	}
	// Check if doesn't overlap with any of the existing modules
	const auto incoming_pos = mark::vector<unsigned>(spos);
	const auto incoming_border = incoming_pos + incoming->size();
	for (const auto i : enumerate(static_cast<unsigned>(m_modules.size()))) {
		if (const auto& module = m_modules[i]) {
			const auto module_pos = mark::vector<unsigned>(i % 16, i / 16);
			const auto module_border = module_pos + module->size();
			if (overlaps(
				{ incoming_pos, incoming_border },
				{ module_pos, module_border })) {
				return mark::error::code::occupied;
			}
		}
	}
	m_modules[incoming_pos.y * 16 + incoming_pos.x] = std::move(incoming);
	return mark::error::code::success;
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
	for (const auto i : mark::enumerate(m_modules.size())) {
		const auto si = static_cast<int>(i);
		const mark::vector<int> module_pos(si % 16, si / 16);
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

void mark::module::cargo::render_contents(mark::vector<double> pos_in, mark::tick_context& context) {
	const auto image = parent().world().resource_manager().image("grid-background.png");
	const auto size = this->interior_size();
	for (const auto point : mark::enumerate(size)) {
		const auto slot_pos = pos_in + mark::vector<double>(point * 16);
		mark::sprite::info info;
		info.image = image;
		info.pos = slot_pos;
		context.sprites[100].emplace_back(info);
		const auto& module = m_modules[point.x + point.y * 16].get();
		if (module) {
			const auto module_pos = slot_pos + mark::vector<double>(module->size()) * 16.0 / 2.0 - mark::vector<double>(8, 8);
			const auto size = static_cast<float>(std::max(module->size().x, module->size().y)) * 16.f;
			mark::sprite::info info;
			info.image = module->thumbnail();
			info.pos = module_pos;
			info.size = size;
			context.sprites[100].emplace_back(info);
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

mark::error::guard mark::module::cargo::push(std::unique_ptr<mark::module::base>& module) {
	for (const auto i : enumerate(static_cast<unsigned>(m_modules.size()))) {
		mark::vector<int> drop_pos(i % 16, i / 16);
		if (this->drop(drop_pos, module) == mark::error::code::success) {
			return mark::error::code::success;
		}
	}
	return mark::error::code::occupied;
}

mark::module::cargo::cargo(mark::resource::manager& rm, const YAML::Node& node):
	mark::module::base(rm, node),
	m_im_body(rm.image("cargo.png")),
	m_im_light(rm.image("glare.png")),
	m_lfo(0.5f, rm.random(0.f, 6.f)),
	m_modules(64) {
	for (const auto& slot_node : node["contents"]) {
		const auto slot = slot_node["slot"].as<size_t>();
		auto module = mark::module::deserialize(rm, slot_node["item"]);
		m_modules[slot] = std::move(module);
	}
}


void mark::module::cargo::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;

	this->serialize_base(out);

	out << Key << "contents" << Value << BeginSeq;
	for (size_t i = 0, size = m_modules.size(); i < size; i++) {
		const auto& module = m_modules[i];
		if (module) {
			out << BeginMap;
			out << Key << "slot" << Value << i;
			out << Key << "item" << Value;
			module->serialize(out);
			out << EndMap;
		}
	}
	out << EndSeq;

	out << EndMap;
}
