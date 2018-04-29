#include "cargo.h"
#include "stdafx.h"
#include <algorithm.h>
#include <exception.h>
#include <resource_manager.h>
#include <sprite.h>
#include <sstream>
#include <unit/bucket.h>
#include <update_context.h>
#include <world.h>

// Serialize / Deserialize

template <typename prop_man, typename T>
void mark::module::cargo::bind(prop_man& property_manager, T& instance)
{
	(void)property_manager;
	(void)instance;
}

mark::module::cargo::cargo(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_im_body(rm.image("cargo.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_im_light(rm.image("glare.png"))
	, m_lfo(0.5f, rm.random(0.f, 6.f))
	, m_items(64)
{
	for (let& slot_node : node["contents"]) {
		let slot = slot_node["slot"].as<size_t>();
		m_items[slot] = module::deserialize(rm, slot_node["item"]);
	}
}

void mark::module::cargo::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;

	base::serialize(out);

	out << Key << "contents" << Value << BeginSeq;
	for (size_t i = 0, size = m_items.size(); i < size; i++) {
		let& module = m_items[i];
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

auto mark::module::cargo::passive() const noexcept -> bool { return true; }

void mark::module::cargo::update(update_context& context)
{
	this->module::base::update(context);
	m_lfo.update(context.dt);
	let pos = this->pos();
	let light_offset = rotate(vd(24.f, 8.f), parent_rotation());
	let light_strength =
		gsl::narrow_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_im_body;
		_.pos = pos;
		_.size = 64.f;
		_.rotation = parent_rotation();
		_.color = this->heat_color();
		return _;
	}());
	context.sprites[4].emplace_back([&] {
		sprite _;
		_.image = m_im_light;
		_.pos = pos + light_offset;
		_.size = 32.f;
		_.rotation = parent_rotation();
		_.color = { 255, 200, 150, light_strength };
		return _;
	}());
	context.sprites[4].emplace_back([&] {
		sprite _;
		_.image = m_im_light;
		_.pos = pos + light_offset;
		_.size = 16.f;
		_.rotation = parent_rotation();
		_.color = { 255, 200, 150, light_strength };
		context.sprites[4].emplace_back(_);
		return _;
	}());
}

auto mark::module::cargo::items() -> std::vector<interface::item_ptr>&
{
	return m_items;
}

auto mark::module::cargo::items() const
	-> const std::vector<interface::item_ptr>&
{
	return m_items;
}

namespace {

auto overlaps(
	const std::pair<mark::vu32, mark::vu32>& left,
	const std::pair<mark::vu32, mark::vu32>& right) -> bool
{
	return left.first.x < right.second.x && left.second.x > right.first.x
		&& left.first.y < right.second.y && left.second.y > right.first.y;
}
} // namespace

auto mark::module::cargo::attach(vi32 pos, interface::item_ptr& item)
	-> std::error_code
{
	{
		let incoming_pos = vu32(pos);
		let incoming_border = incoming_pos + item->size();
		for (let[index, cur_item] : enumerate(m_items)) {
			if (!cur_item)
				continue;
			let i = gsl::narrow<unsigned>(index);
			let item_pos = vu32(i % 16, i / 16);
			let item_border = item_pos + cur_item->size();
			if (overlaps(
					{ incoming_pos, incoming_border },
					{ item_pos, item_border })
				&& cur_item->can_stack(*item)) {
				cur_item->stack(item);
				return error::code::stacked;
			}
		}
	}
	if (!this->can_attach(pos, *item)) {
		return error::code::bad_pos;
	}
	m_items[pos.y * 16 + pos.x] = std::move(item);
	return error::code::success;
}

auto mark::module::cargo::can_attach(vi32 pos, const interface::item& item)
	const -> bool
{
	// Check if fits inside the container
	let cargo_size = vector<size_t>(16, m_items.size() / 16);
	if (pos.x < 0 || pos.y < 0 || pos.x + item.size().x > cargo_size.x
		|| pos.y + item.size().y > cargo_size.y) {
		return false;
	}
	// Check if doesn't overlap with any of the existing modules
	let incoming_pos = vu32(pos);
	let incoming_border = incoming_pos + item.size();
	for (let pair : enumerate(m_items)) {
		if (let& cur_item = pair.second) {
			let i = gsl::narrow<unsigned>(pair.first);
			let item_pos = vu32(i % 16, i / 16);
			let item_border = item_pos + cur_item->size();
			if (overlaps(
					{ incoming_pos, incoming_border },
					{ item_pos, item_border })) {
				return cur_item->can_stack(item);
			}
		}
	}
	return true;
}

auto mark::module::cargo::at(vi32 pos) -> interface::item*
{
	return const_cast<interface::item*>(
		static_cast<const module::cargo*>(this)->at(pos));
}

auto mark::module::cargo::at(vi32 i_pos) const -> const interface::item*
{
	if (let pos = this->pos_at(i_pos)) {
		return m_items[pos->x + pos->y * 16].get();
	}
	return nullptr;
}

auto mark::module::cargo::pos_at(vi32 i_pos) const noexcept
	-> std::optional<vi32>
{
	if (i_pos.x < 0 || i_pos.y < 0) {
		return {};
	}
	let pos = vu32(i_pos);
	for (let & [ i, slot ] : enumerate(m_items)) {
		if (!slot) {
			continue;
		}
		let item_pos = modulo_vector<uint32_t>(gsl::narrow<int32_t>(i), 16LU);
		let border = item_pos + slot->size();
		if (pos.x >= item_pos.x && pos.x < border.x && pos.y >= item_pos.y
			&& pos.y < border.y) {
			return vi32(item_pos);
		}
	}
	return {};
}

auto mark::module::cargo::detach(vi32 i_pos) -> interface::item_ptr
{
	if (let pos = this->pos_at(i_pos)) {
		return move(m_items[pos->x + pos->y * 16]);
	}
	return nullptr;
}

auto mark::module::cargo::interior_size() const -> vi32
{
	return { 16, gsl::narrow<int>(m_items.size()) / 16 };
}

auto mark::module::cargo::detachable() const -> bool
{
	for (let& item : m_items) {
		if (item) {
			return false;
		}
	}
	return true;
}

std::string mark::module::cargo::describe() const
{
	std::ostringstream os;
	os << "Cargo Module" << std::endl;
	os << "Capacity: " << m_items.size() << std::endl;
	return os.str();
}

void mark::module::cargo::on_death(update_context& context)
{
	module::base::on_death(context);
	for (auto& item : m_items) {
		if (item) {
			context.units.push_back(std::make_shared<unit::bucket>([&] {
				unit::bucket::info info;
				info.world = &world();
				info.pos = pos();
				info.item = std::move(item);
				return info;
			}()));
		}
	}
}

auto mark::module::cargo::push(interface::item_ptr& item) -> std::error_code
{
	for (let i : range(gsl::narrow<int>(m_items.size()))) {
		let drop_pos = modulo_vector<int>(i, 16);
		let result = this->attach(drop_pos, item);
		if (result == error::code::success
			|| result == error::code::stacked && !item) {
			return error::code::success;
		}
	}
	return error::code::occupied;
}
