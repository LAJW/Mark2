﻿#include "modular.h"
#include <algorithm.h>
#include <command.h>
#include <exception.h>
#include <map.h>
#include <module/base.h>
#include <module/cargo.h>
#include <module/core.h>
#include <module/shield_generator.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <targeting_system.h>
#include <unit/activable.h>
#include <unit/bucket.h>
#include <update_context.h>
#include <world.h>

// MODULAR

using namespace mark;
namespace {

let constexpr knockback_duration = .15;

// Map from modular coordinates (relative to center) to grid coordinates
// (relative to top left corner)
auto to_grid(vi8 pos8) -> vector<size_t>
{
	let pos = vector<size_t>(pos8);
	let hs = mark::unit::modular::max_size / 2;
	return { hs + pos.x, hs + pos.y };
}

struct Node
{
	vi8 pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

bool operator<(const Node& left, const Node& right) { return left.f < right.f; }

// Shared implementation of the modular::neighbors_of() function
// Returns list of references to modules and counts, where count is the
// number of blocks touching the modules
template <typename module_t, typename modular_t>
static auto neighbors_of(modular_t& modular, vi8 pos, vi8 size)
{
	std::vector<std::pair<ref<module_t>, unsigned>> out;
	auto out_insert = [&out](module_t* module_ptr) {
		if (module_ptr) {
			if (out.empty() || &out.back().first.get() != module_ptr) {
				out.push_back({ *module_ptr, 1U });
			} else {
				++out.back().second;
			}
		}
	};
	// right
	for (let i : mark::range(size.y)) {
		auto module_ptr = modular.module_at({ pos.x + size.x, pos.y + i });
		out_insert(module_ptr);
	}
	// bottom
	for (let i : mark::range(size.x)) {
		auto module_ptr = modular.module_at({ pos.x + i, pos.y + size.y });
		out_insert(module_ptr);
	}
	// left
	for (let i : mark::range(size.y)) {
		auto module_ptr = modular.module_at({ pos.x - 1, pos.y + i });
		out_insert(module_ptr);
	}
	// top
	for (let i : mark::range(size.x)) {
		auto module_ptr = modular.module_at({ pos.x + i, pos.y - 1 });
		out_insert(module_ptr);
	}
	return out;
}

template <typename module_type, typename vector_type>
static auto filter_modules(vector_type& modules)
{
	std::vector<ref<module_type>> out;
	for (let& module : modules) {
		if (let derived = dynamic_cast<module_type*>(module.get())) {
			out.push_back(std::ref(*derived));
		}
	}
	return out;
}
} // namespace

mark::unit::modular::modular(info info)
	: unit::mobile(info)
	, m_rotation(info.rotation)
	, m_targeting_system(std::make_unique<mark::targeting_system>(*this))
{}

mark::unit::modular::~modular() = default;

void mark::unit::modular::update_modules(update_context& context)
{
	this->remove_dead(context);
	for (auto& module : m_modules) {
		// Module might be already dead, don't update dead modules
		if (!module->dead()) {
			static_cast<module::base_ref&>(*module).update(context);
		}
	}
}

std::vector<mark::command::any> mark::unit::modular::update_ai() const
{
	let enemy = world().find_one<unit::damageable>(
		this->pos(), 1000.f, [this](let& unit) {
			return unit.team() != this->team();
		});
	if (!enemy) {
		return {};
	}
	let delta = enemy->pos() - this->pos();
	let distance_to_enemy = length(delta);
	let minimal_distance = std::max(0., this->radius() + enemy->radius());
	if (distance_to_enemy <= minimal_distance) {
		return { command::guide{ enemy->pos() }, command::move{ this->pos() } };
	}
	let direction = delta / distance_to_enemy;
	let target_pos = enemy->pos() - direction * minimal_distance;
	return { command::guide{ enemy->pos() }, command::move{ target_pos } };
}

auto mark::unit::modular::p_connected_to_core(const module::base& module) const
	-> bool
{
	let size = gsl::narrow<int8_t>(m_grid.size().x);
	let hs = int8_t(size / 2);
	let start = vi8(module.grid_pos()) + vi8(hs, hs);
	let end = vi8(size, size) / int8_t(2);
	std::vector<Node> open = { Node{
		start, static_cast<int>(length(end - start)), nullptr } };
	std::vector<unique_ptr<Node>> closed;

	while (!open.empty()) {
		auto min_it = min_element(open.begin(), open.end());
		closed.push_back(std::make_unique<Node>(*min_it));
		auto& current = closed.back();
		// TODO replace with the last element and pop back, instead of erase
		open.erase(min_it);

		if (current->pos == end) {
			return true;
		}

		// TODO Replace with int8_t, and remove vector casts
		// TODO Replace with range
		for (int i = 1; i < 8; i += 2) {
			auto neighbour_pos =
				current->pos + vi8(i % 3 - 1, gsl::narrow<int8_t>(i / 3 - 1));
			let traversable = neighbour_pos.x > 0 && neighbour_pos.y > 0
				&& neighbour_pos.x < size && neighbour_pos.y < size
				&& m_grid[vector<size_t>(neighbour_pos)].module;
			let isClosed = closed.end()
				!= find_if(closed.cbegin(),
						   closed.cend(),
						   [&neighbour_pos](let& node) {
							   return node->pos == neighbour_pos;
						   });
			if (!traversable || isClosed) {
				continue;
			}
			auto neighbour_it =
				find_if(open.begin(), open.end(), [&neighbour_pos](let& node) {
					return neighbour_pos == node.pos;
				});
			let f = current->f + 10;
			if (neighbour_it == open.end()) {
				open.push_back({ neighbour_pos, f, current.get() });
			} else if (neighbour_it->f > f) {
				neighbour_it->f = f;
				neighbour_it->parent = current.get();
			}
		}
	}
	return false;
}

auto mark::unit::modular::p_at(vi8 user_pos) noexcept -> grid_element&
{
	return m_grid[to_grid(user_pos)];
}

auto mark::unit::modular::p_at(vi8 user_pos) const noexcept
	-> const grid_element&
{
	return m_grid[to_grid(user_pos)];
}

void mark::unit::modular::ai(bool ai) { m_ai = ai; }

auto mark::unit::modular::radius() const -> double { return m_radius; }

auto mark::unit::modular::targeting_system() const
	-> const mark::targeting_system&
{
	return *m_targeting_system;
}

auto mark::unit::modular::targeting_system() -> mark::targeting_system&
{
	return *m_targeting_system;
}

auto mark::unit::modular::modifiers() const -> module::modifiers
{
	module::modifiers mods;
	for (auto& module : m_modules) {
		let cur_mod = module->global_modifiers();
		mods.velocity += cur_mod.velocity;
		mods.mass += cur_mod.mass;
	}
	return mods;
}

namespace mark {
// Given velocity and path (path starts at "current" location, so it's whatever
// pathfinder returns with "pos" added to the front) return remaining path
// starting at new pos (pos reached after ascending the path in time limited by
// delta time)
static auto advance_path(std::vector<vd> path, double velocity, double dt)
	-> std::vector<vd>
{
	double remaining_distance = velocity * dt;
	size_t final_i = 0;
	vd final_delta;
	for (let[i, cur] : enumerate(path)) {
		if (i == 0) {
			continue;
		}
		let prev = path[i - 1];
		let cur_length = length(cur - prev);
		final_delta = normalize(cur - prev) * remaining_distance;
		final_i = i - 1;
		remaining_distance -= cur_length;
		if (remaining_distance <= 0.) {
			break;
		}
	}
	if (remaining_distance < 0.) {
		path.erase(path.begin(), path.begin() + final_i);
		path.front() += final_delta;
		return path;
	}
	return { path.back() };
}
} // namespace mark

void mark::unit::modular::update(update_context& context)
{
	let modifiers = this->modifiers();
	m_targeting_system->update(context);
	this->update_modules(context);
	if (!m_ai && world().target().get() == this) {
		this->pick_up();
	}
	if (m_knockback_path.size() > 1 && m_initial_knockback_path_length >= 0.) {
		let velocity = m_initial_knockback_path_length / knockback_duration;
		m_knockback_path =
			advance_path(move(m_knockback_path), velocity, context.dt);
		pos(m_knockback_path.front());
	} else {
		m_initial_knockback_path_length = 0.; // Unlock knockback retrigger
		this->update_movement([&] {
			update_movement_info _;
			_.ai = m_ai;
			_.max_velocity = m_ai ? 64.0 : 320.0 + modifiers.velocity;
			_.acceleration =
				modifiers.mass > 1.f ? 5000.f / modifiers.mass : 500.f;
			_.dt = context.dt;
			return _;
		}());
	}
	if (m_lookat != pos()) {
		let turn_speed = m_ai ? 32.f : 128.f;
		let target = m_targeting_system->target();
		let lookat = target ? *target : m_lookat;
		m_rotation = turn(lookat - pos(), m_rotation, turn_speed, context.dt);
	}
	if (m_ai) {
		for (let& command : this->update_ai()) {
			this->command(command);
		}
	}
}

auto mark::unit::modular::neighbors_of(const module::base& module)
	-> std::vector<std::pair<ref<module::base>, unsigned>>
{
	return ::neighbors_of<module::base>(
		*this, vi8(module.grid_pos()), vi8(module.size()));
}

auto mark::unit::modular::neighbors_of(const module::base& module) const
	-> std::vector<std::pair<cref<module::base>, unsigned>>
{
	return ::neighbors_of<const module::base>(
		*this, vi8(module.grid_pos()), vi8(module.size()));
}

auto mark::unit::modular::attach(vi32 pos_, interface::item_ptr& item)
	-> std::error_code
{
	let module_pos = vi8(pos_);
	if (!item || !can_attach(pos_, *item)) {
		return error::code::bad_pos;
	}
	module::base_ptr module(dynamic_cast<module::base*>(item.release()));
	return p_attach(pos_, module);
}

auto mark::unit::modular::p_attach(vi32 pos_, module::base_ptr& module)
	-> std::error_code
{
	let module_pos = vi8(pos_);
	if (!module || !p_can_attach(*module, pos_)) {
		return error::code::bad_pos;
	}
	// establish core, check if core already present
	if (auto core = dynamic_cast<module::core*>(module.get())) {
		m_core = core;
	}
	module->m_grid_pos = module_pos;
	module->m_parent = this;
	for (let i : range(module->size())) {
		this->p_at(module_pos + vi8(i)).module = module.get();
	}
	if (module->reserved() == module::reserved_kind::back) {
		for (let i : range<vi32>(
				 { -gsl::narrow<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + gsl::narrow<int>(module->size().y) })) {
			this->p_at(vi8(i)).reserved = true;
		}
	} else if (module->reserved() == module::reserved_kind::front) {
		for (let i : range<vi32>(
				 { module_pos.x + gsl::narrow<int>(module->size().x),
				   module_pos.y },
				 { gsl::narrow<int>(max_size / 2),
				   module_pos.y + gsl::narrow<int>(module->size().y) })) {
			this->p_at(vi8(i)).reserved = true;
		}
	}
	m_radius = std::max(
		m_radius, length(module->pos() - this->pos())
		/* + length(module->size()) * static_cast<double>(module::size) / 2. */);
	m_modules.emplace_back(move(module));
	return error::code::success;
}

auto mark::unit::modular::can_attach(vi32 pos_, const interface::item& item)
	const -> bool
{
	let module = dynamic_cast<const module::base*>(&item);
	if (!module) {
		return false;
	}
	return p_can_attach(*module, pos_)
		&& (m_modules.empty()
			|| !::neighbors_of<const module::base>(
					*this, vi8(pos_), vi8(module->size()))
					.empty());
}

auto mark::unit::modular::p_can_attach(const module::base& module, vi32 pos_)
	const -> bool
{
	if (!(pos_.x >= -19 && pos_.x + gsl::narrow<int>(module.size().x) < 19
		  && pos_.y >= -19
		  && pos_.y + gsl::narrow<int>(module.size().y) < 19)) {
		return false;
	}
	let module_pos = vi8(pos_);
	for (let i : range(vi8(module.size()))) {
		let[module_ptr, reserved] = this->p_at(module_pos + i);
		if (module_ptr || reserved) {
			return false;
		}
	}
	// Establish core, check if core already present
	if (dynamic_cast<const module::core*>(&module) && m_core) {
		return false;
	}
	if (module.reserved() == module::reserved_kind::back) {
		for (let i : range<vi32>(
				 { -gsl::narrow<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + gsl::narrow<int>(module.size().y) })) {
			if (this->p_at(vi8(i)).module) {
				return false;
			}
		}
	} else if (module.reserved() == module::reserved_kind::front) {
		for (let i : range<vi32>(
				 { module_pos.x + gsl::narrow<int>(module.size().x),
				   module_pos.y },
				 { gsl::narrow<int>(max_size / 2),
				   module_pos.y + gsl::narrow<int>(module.size().y) })) {
			if (this->p_at(vi8(i)).module) {
				return false;
			}
		}
	}
	return true;
}

auto mark::unit::modular::detach(vi32 user_pos) -> interface::item_ptr
{

	let module_ptr = this->module_at(user_pos);
	if (!module_ptr) {
		return nullptr;
	}
	auto& module = *module_ptr;
	if (!module.detachable()) {
		return nullptr;
	}
	let module_pos = vi8(module.grid_pos());
	let module_size = vi8(module.size());
	// remove module from the grid
	let surface = range(module_pos, module_pos + module_size);
	for (let grid_pos : surface) {
		this->p_at(grid_pos).module = nullptr;
	}
	let neighbors = this->neighbors_of(module);
	let disconnected =
		find_if(neighbors.begin(), neighbors.end(), [this](let& neighbour) {
			return !this->p_connected_to_core(neighbour.first.get());
		});
	if (disconnected != neighbors.end()) {
		for (let grid_pos : surface) {
			this->p_at(grid_pos).module = module_ptr;
		}
		return nullptr;
	}
	if (module.reserved() == module::reserved_kind::back) {
		for (let i : range<vi32>(
				 { -gsl::narrow<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + gsl::narrow<int>(module.size().y) })) {
			this->p_at(vi8(i)).reserved = false;
		}
	} else if (module.reserved() == module::reserved_kind::front) {
		for (let i : range<vi32>(
				 { module_pos.x + module_size.x, module_pos.y },
				 { gsl::narrow<int>(max_size / 2),
				   module_pos.y + gsl::narrow<int>(module.size().y) })) {
			this->p_at(vi8(i)).reserved = false;
		}
	}
	this->unbind(module);
	let module_it = find_if(m_modules.begin(), m_modules.end(), [=](let& ptr) {
		return ptr.get() == module_ptr;
	});
	m_radius = 0.;
	for (let& cur_module : m_modules) {
		m_radius = std::max(m_radius, length(cur_module->pos() - this->pos()));
	}
	return move(drop(m_modules, module_it));
}

namespace {
static auto ability_id(const mark::command::any& any) -> std::optional<int8_t>
{
	using namespace mark;
	if (let release = std::get_if<command::release>(&any)) {
		return release->id;
	}
	if (let activate = std::get_if<command::activate>(&any)) {
		return activate->id;
	}
	if (let queue = std::get_if<command::queue>(&any)) {
		return queue->id;
	}
	return {};
}
} // namespace

void mark::unit::modular::command(const command::any& any)
{
	if (dead()) {
		// TODO Propagate error
		return;
	}
	mobile::command(any);
	if (let guide = std::get_if<command::guide>(&any)) {
		m_lookat = guide->pos;
		for (auto& module : m_modules) {
			module->command(command::guide{ guide->pos });
		}
	} else if (std::holds_alternative<command::use>(any)) {
		if (let pad = world().find_one<activable>(pos(), 150.0)) {
			// TODO Propagate error
			(void)pad->use(
				std::dynamic_pointer_cast<modular>(this->shared_from_this()));
		}
	} else if (let id = ability_id(any)) {
		auto bindings = m_bindings.equal_range(*id);
		for_each(bindings.first, bindings.second, [&](auto module) {
			module.second.get().command(any);
		});
	}
}

auto mark::unit::modular::dead() const -> bool
{
	return !m_core || m_core->dead();
}

void mark::unit::modular::on_death(update_context& context)
{
	for (auto& module : m_modules) {
		if (module->dead()) {
			continue;
		}
		context.units.emplace_back(std::make_shared<unit::bucket>([&] {
			unit::bucket::info info;
			info.world = this->world();
			info.pos = module->pos();
			info.rotation = rotation();
			info.item = move(module);
			return info;
		}()));
	}
}

bool mark::unit::modular::damage(const interface::damageable::info& attr)
{
	for (auto& module : m_modules) {
		if (module->damage(attr)) {
			attr.damaged->insert(this);
			return true;
		}
	}
	return false;
}

namespace mark {
static auto path_length(const std::vector<vd>& path)
{
	double path_length = 0.0;
	for (let[i, cur] : enumerate(path)) {
		if (i > 0) {
			let prev = path[i - 1];
			let cur_length = length(prev - cur);
			path_length += cur_length;
		}
	}
	Ensures(path_length >= 0.);
	return path_length;
}
} // namespace mark

void mark::unit::modular::knockback(
	std::unordered_set<not_null<interface::damageable*>>& knocked,
	float angle,
	double distance)
{
	if (distance <= m_initial_knockback_path_length
		|| !knocked.insert(this).second) {
		return;
	}
	const auto new_pos = pos() + rotate(vd(distance, 0.), angle);
	auto path = world().map().find_path(pos(), new_pos, this->radius());
	if (path.size() == 1) {
		path = { pos(), path.back() };
	}
	m_initial_knockback_path_length = path_length(path);
	m_knockback_path = move(path);
}

auto mark::unit::modular::collide(const segment_t& ray)
	-> std::optional<std::pair<ref<interface::damageable>, vd>>
{
	std::optional<vd> min;
	double min_length = INFINITY;
	interface::damageable* damageable = nullptr;
	for (auto& module : m_modules) {
		if (let result = module->collide(ray)) {
			let length = mark::length(ray.first - result->second);
			if (length < min_length) {
				min_length = length;
				min = result->second;
				damageable = &result->first.get();
			}
		}
	}
	if (!min) {
		return {};
	}
	// check if module is under the shield
	for (let shield : filter_modules<module::shield_generator>(m_modules)) {
		if (shield.get().shield() > 0.f) {
			let shield_pos = shield.get().pos();
			let shield_size = 64.f;
			if (length(*min - shield_pos) < shield_size - 1.f) {
				return {};
			}
		}
	}
	return { { std::ref(*damageable), *min } };
}

auto mark::unit::modular::collide(vd center, double radius)
	-> std::vector<ref<interface::damageable>>
{
	std::unordered_set<interface::damageable*> out;
	let shields = filter_modules<module::shield_generator>(m_modules);
	for (auto& module : m_modules) {
		let module_size = std::max(module->size().x, module->size().y);
		let module_pos = module->pos();
		if (length(center - module_pos) < module_size + radius) {
			for (auto shield : shields) {
				if (shield.get().shield() > 0.f) {
					let shield_pos = shield.get().pos();
					let shield_size = 64.f;
					if (length(module_pos - shield_pos) < shield_size) {
						out.insert(&shield.get());
						goto outer_continue;
					}
				}
			}
			out.insert(module.get());
		}
	outer_continue:;
	}
	std::vector<ref<interface::damageable>> tmp;
	transform(out.cbegin(), out.cend(), back_inserter(tmp), [](let ptr) {
		return std::ref(*ptr);
	});
	return tmp;
}

void mark::unit::modular::toggle_bind(int8_t command_id, vi32 user_pos)
{
	if (let module_ptr = this->module_at(user_pos)) {
		auto& module = *module_ptr;
		if (module.passive()) {
			return;
		}
		let bindings = m_bindings.equal_range(command_id);
		let binding =
			find_if(bindings.first, bindings.second, [&module](auto pair) {
				return &module == &pair.second.get();
			});
		if (binding == bindings.second) {
			m_bindings.insert({ command_id, module });
		} else {
			m_bindings.erase(binding);
		}
	}
}

auto mark::unit::modular::binding(vi32 user_pos) const -> std::vector<int8_t>
{
	std::vector<int8_t> out;
	if (let module = this->at(user_pos)) {
		for (let& pair : m_bindings) {
			if (module == &pair.second.get()) {
				out.push_back(pair.first);
			}
		}
	}
	return out;
}

auto mark::unit::modular::bindings() const -> modular::bindings_t
{
	modular::bindings_t out;
	for (let& binding : m_bindings) {
		let & [ command_id, module ] = binding;
		out[command_id].total++;
		out[command_id].thumbnail = module.get().thumbnail();
		out[command_id].modules.push_back(module.get());
	}
	return out;
}

// Serializer / Deserializer

mark::unit::modular::modular(mark::world& world, const YAML::Node& node)
	: unit::mobile(world, node)
	, m_ai(node["ai"].as<bool>())
	, m_targeting_system(std::make_unique<mark::targeting_system>(*this))
{
	std::unordered_map<uint64_t, ref<module::base>> id_map;
	for (let& module_node : node["modules"]) {
		let module_pos = module_node["grid_pos"].as<vi32>();
		let id = module_node["id"].as<uint64_t>();
		auto item = [&] {
			let blueprint_node = module_node["blueprint"];
			auto& rm = world.resource_manager();
			if (!blueprint_node) {
				return module::deserialize(rm, module_node);
			}
			let blueprint_id = [&] {
				if (blueprint_node.IsSequence()) {
					if (blueprint_node.size() == 0) {
						throw std::runtime_error("Empty blueprint selection");
					}
					let index = rm.random<size_t>(0, blueprint_node.size() - 1);
					return blueprint_node[index].as<std::string>();
				}
				return blueprint_node.as<std::string>();
			}();
			auto properties = world.blueprints().at(blueprint_id);
			for (let& property : module_node) {
				properties[property.first] = property.second;
			}
			return module::deserialize(rm, properties);
		}();
		module::base_ptr module(dynamic_cast<module::base*>(item.release()));
		assert(module);
		if (this->p_attach(module_pos, module) != error::code::success) {
			throw exception("BAD_MODULE_POS");
		}
		id_map.insert({ id, *m_modules.back() });
	}
	for (let& binding_node : node["bindings"]) {
		let key = gsl::narrow<int8_t>(binding_node["key"].as<int>());
		let module_id = binding_node["module_id"].as<uint64_t>();
		m_bindings.insert({ key, id_map.at(module_id) });
	}
}

void mark::unit::modular::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	mobile::serialize(out);

	out << Key << "type" << Value << unit::modular::type_name;

	out << Key << "modules" << Value << BeginSeq;
	for (let& module : m_modules) {
		module->serialize(out);
	}
	out << EndSeq;

	out << Key << "lookat" << Value << BeginMap;
	out << Key << "x" << Value << m_lookat.x;
	out << Key << "y" << Value << m_lookat.y;
	out << EndMap;

	out << Key << "ai" << Value << m_ai;

	out << Key << "bindings" << Value << BeginSeq;

	for (let& pair : m_bindings) {
		out << BeginMap;
		out << Key << "key" << Value << gsl::narrow<int>(pair.first);
		out << Key << "module_id"
			<< reinterpret_cast<size_t>(&pair.second.get());
		out << EndMap;
	}
	out << EndSeq;

	// base
	out << EndMap;
}

auto mark::unit::modular::at(vi32 module_pos) noexcept -> interface::item*
{
	return module_at(module_pos);
}

auto mark::unit::modular::at(vi32 module_pos) const noexcept
	-> const interface::item*
{
	return module_at(module_pos);
}

auto mark::unit::modular::pos_at(vi32 pos) const noexcept -> std::optional<vi32>
{
	if (let module = module_at(pos)) {
		return module->grid_pos();
	}
	return {};
}

auto mark::unit::modular::module_at(vi32 pos) noexcept -> module::base*
{
	let hs = gsl::narrow<int8_t>(max_size / 2);
	if (pos.x >= -hs && pos.y < hs) {
		return this->p_at(vi8(pos)).module;
	}
	return nullptr;
}

auto mark::unit::modular::module_at(vi32 pos) const noexcept
	-> const module::base*
{
	let hs = gsl::narrow<int8_t>(max_size / 2);
	if (pos.x >= -hs && pos.y < hs) {
		return this->p_at(vi8(pos)).module;
	}
	return nullptr;
}

auto mark::unit::modular::landed() const noexcept -> bool
{
	return !m_ai && world().target().get() != this;
}

void mark::unit::modular::remove_dead(update_context& context)
{
	let first_dead_it = partition(
		m_modules.begin(), m_modules.end(), [](const module::base_ptr& module) {
			return !module->dead();
		});
	if (first_dead_it != m_modules.end()) {
		for_each(
			first_dead_it,
			m_modules.end(),
			[this, &context](module::base_ptr& module) {
				module->on_death(context);
				if (module.get() == m_core) {
					m_core = nullptr;
				}
				this->unbind(*module);
				let module_pos = vi8(module->grid_pos());
				let module_size = vi8(module->size());
				for (let i : range(module_size)) {
					this->p_at(module_pos + i).module = nullptr;
				}
			});
		let first_detached_it =
			partition(m_modules.begin(), first_dead_it, [this](let& module) {
				return this->p_connected_to_core(*module);
			});
		for_each(first_detached_it, m_modules.end(), [this](let& module) {
			this->unbind(*module);
			let module_pos = vi8(module->grid_pos());
			let module_size = vi8(module->size());
			for (let i : range(module_size)) {
				this->p_at(module_pos + i).module = nullptr;
			}
		});
		transform(
			make_move_iterator(first_detached_it),
			make_move_iterator(first_dead_it),
			back_inserter(context.units),
			[this](auto module) {
				unit::bucket::info info;
				info.world = this->world();
				info.pos = this->pos();
				info.item = move(module);
				return std::make_shared<unit::bucket>(std::move(info));
			});
		m_modules.erase(first_detached_it, m_modules.end());
	}
}

void mark::unit::modular::pick_up()
{
	auto buckets = world().find<unit::bucket>(
		pos(), 150.f, [](let& unit) { return !unit.dead(); });
	for (auto& bucket : buckets) {
		auto module = bucket->release();
		if (push(*this, module) != error::code::success) {
			bucket->insert(move(module));
		}
	}
}

void mark::unit::modular::unbind(const module::base& module)
{
	auto it = m_bindings.begin();
	let end = m_bindings.end();
	while (it != end) {
		auto cur = it;
		it++;
		if (&cur->second.get() == &module) {
			m_bindings.erase(cur);
		}
	}
}

auto mark::unit::modular::containers() -> std::vector<ref<module::cargo>>
{
	return filter_modules<module::cargo>(m_modules);
}

auto mark::unit::modular::containers() const -> std::vector<cref<module::cargo>>
{
	return filter_modules<const module::cargo>(m_modules);
}

auto mark::unit::push(modular& modular, interface::item_ptr& module)
	-> std::error_code
{
	for (auto& container : modular.containers()) {
		if (container.get().push(module) == error::code::success) {
			return error::code::success;
		}
	}
	return error::code::occupied;
}
