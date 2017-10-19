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

mark::module::cargo::cargo(resource::manager& resource_manager)
	: module::base({ 4, 2 }, resource_manager.image("cargo.png"))
	, m_grid_bg(resource_manager.image("grid-background.png"))
	, m_im_body(resource_manager.image("cargo.png"))
	, m_im_light(resource_manager.image("glare.png"))
	, m_lfo(0.5f, resource_manager.random(0.f, 6.f))
	, m_modules(64) { }

// Serialize / Deserialize

mark::module::cargo::cargo(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_im_body(rm.image("cargo.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_im_light(rm.image("glare.png"))
	, m_lfo(0.5f, rm.random(0.f, 6.f))
	, m_modules(64)
{
	for (const auto& slot_node : node["contents"]) {
		const auto slot = slot_node["slot"].as<size_t>();
		auto module = module::deserialize(rm, slot_node["item"]);
		m_modules[slot] = std::move(module);
	}
}


void mark::module::cargo::serialize(YAML::Emitter& out) const
{
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

auto mark::module::cargo::passive() const noexcept -> bool
{ return true; }

void mark::module::cargo::tick(tick_context& context)
{
	this->module::base::tick(context);
	m_lfo.tick(context.dt);
	auto pos = this->pos();
	auto light_offset = rotate(vector<double>(24.f, 8.f), parent().rotation());
	auto light_strength = static_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	{
		sprite info;
		info.image = m_im_body;
		info.pos = pos;
		info.size = 64.f;
		info.rotation = parent().rotation();
		info.color = this->heat_color();
		context.sprites[2].emplace_back(info);
	}
	{
		sprite info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 32.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[4].emplace_back(info);
	}
	{
		sprite info;
		info.image = m_im_light;
		info.pos = pos + light_offset;
		info.size = 16.f;
		info.rotation = parent().rotation();
		info.color = { 255, 200, 150, light_strength };
		context.sprites[4].emplace_back(info);
	}
}

auto mark::module::cargo::modules() ->
	std::vector<std::unique_ptr<module::base>>&
{ return m_modules; }

auto mark::module::cargo::modules() const ->
	const std::vector<std::unique_ptr<module::base>>&
{ return m_modules; }

namespace {

auto overlaps(
	const std::pair<mark::vector<unsigned>, mark::vector<unsigned>>& left,
	const std::pair<mark::vector<unsigned>, mark::vector<unsigned>>& right)
	-> bool {
	return left.first.x < right.second.x
		&& left.second.x > right.first.x
		&& left.first.y < right.second.y
		&& left.second.y > right.first.y;
}

}

auto mark::module::cargo::attach(
	const vector<int>& pos,
	std::unique_ptr<module::base>& module) -> std::error_code
{
	if (!this->can_attach(pos, *module)) {
		return error::code::bad_pos;
	}
	module->m_grid_pos = vector<int8_t>(pos);
	m_modules[pos.y * 16 + pos.x] = std::move(module);
	return error::code::success;
}

auto mark::module::cargo::can_attach(
	const vector<int>& pos, const module::base & module) const -> bool
{
	// Check if fits inside the container
	const auto cargo_size = vector<size_t>(16, m_modules.size() / 16);
	if (pos.x < 0 || pos.y < 0
		|| pos.x + module.size().x > cargo_size.x
		|| pos.y + module.size().y > cargo_size.y) {
		return false;
	}
	// Check if doesn't overlap with any of the existing modules
	const auto incoming_pos = vector<unsigned>(pos);
	const auto incoming_border = incoming_pos + module.size();
	for (const auto pair : enumerate(m_modules)) {
		if (const auto& cur_module = pair.second) {
			const auto i = static_cast<unsigned>(pair.first);
			const auto module_pos = vector<unsigned>(i % 16, i / 16);
			const auto module_border = module_pos + cur_module->size();
			if (overlaps(
				{ incoming_pos, incoming_border },
				{ module_pos, module_border })) {
				return false;
			}
		}
	}
	return true;
}

auto mark::module::cargo::at(const vector<int>& pos) -> module::base*
{
	return const_cast<module::base*>(
		static_cast<const module::cargo*>(this)->at(pos));
}

auto mark::module::cargo::at(const vector<int>& i_pos) const
	-> const module::base*
 {
	if (i_pos.x < 0 || i_pos.y < 0) {
		return nullptr;
	}
	const auto pos = vector<unsigned>(i_pos);
	for (const auto pair : enumerate(m_modules)) {
		const auto module_pos = modulo_vector(pair.first, 16LLU);
		if (const auto& slot = pair.second) {
			const auto border = module_pos + vector<size_t>(slot->size());
			if (pos.x + pos.x >= module_pos.x && pos.x < border.x
				&& pos.y + pos.y >= module_pos.y && pos.y < border.y) {
				return slot.get();
			}
		}
	}
	return nullptr;
}

auto mark::module::cargo::detach(const vector<int>& pos) ->
	std::unique_ptr<module::base>
{
	if (pos.x < 0 && pos.y < 0) {
		return nullptr;
	}
	for (const auto pair : enumerate(m_modules)) {
		const auto i = static_cast<int>(pair.first);
		const auto module_pos = modulo_vector(i, 16);
		if (auto& module = pair.second) {
			const auto border = module_pos + vector<int>(module->size());
			if (pos.x >= module_pos.x && pos.x < border.x
				&& pos.y >= module_pos.y && pos.y < border.y) {
				return std::move(module);
			}
		}
	}
	return nullptr;
}

auto mark::module::cargo::interior_size() const -> vector<int> {
	auto size_v = static_cast<int>(m_modules.size());
	return vector<int>(16, size_v / 16);
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

void mark::module::cargo::on_death(tick_context & context) {
	module::base::on_death(context);
	for (auto& module : m_modules) {
		if (module) {
			context.units.push_back(std::make_shared<unit::bucket>(
				parent().world(),
				pos(),
				std::move(module)));
		}
	}
}

auto mark::module::cargo::push(
	std::unique_ptr<module::base>& module) -> std::error_code
{
	for (const auto i : range(static_cast<int>(m_modules.size()))) {
		const auto drop_pos = modulo_vector(i, 16);
		if (this->attach(drop_pos, module) == error::code::success) {
			return error::code::success;
		}
	}
	return error::code::occupied;
}