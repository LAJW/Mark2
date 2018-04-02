#include "modular.h"
#include "activable.h"
#include "bucket.h"
#include <algorithm.h>
#include <command.h>
#include <exception.h>
#include <module/base.h>
#include <module/cargo.h>
#include <module/core.h>
#include <module/shield_generator.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <tick_context.h>
#include <world.h>

// MODULAR

namespace {

// temporary refactor helper
auto to_new(mark::vector<int8_t> pos) -> mark::vector<uint8_t>
{
	let hs = static_cast<int8_t>(mark::unit::modular::max_size / 2);
	return { static_cast<uint8_t>(hs + pos.x),
			 static_cast<uint8_t>(hs + pos.y) };
}

struct Node
{
	mark::vector<int8_t> pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

bool operator<(const Node& left, const Node& right) { return left.f < right.f; }

// Shared implementation of the modular::neighbours_of() function
// Returns list of references to modules and counts, where count is the
// number of blocks touching the modules
template <typename module_t, typename modular_t>
static auto neighbours_of(
	modular_t& modular,
	mark::vector<int8_t> pos,
	mark::vector<int8_t> size)
{
	std::vector<std::pair<std::reference_wrapper<module_t>, unsigned>> out;
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
	std::vector<std::reference_wrapper<module_type>> out;
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
{}

void mark::unit::modular::tick_modules(tick_context& context)
{
	this->remove_dead(context);
	for (auto& module : m_modules) {
		// Module might be already dead, don't tick dead modules
		if (!module->dead()) {
			static_cast<module::base_ref&>(*module).tick(context);
		}
	}
}

std::vector<mark::command::any> mark::unit::modular::tick_ai() const
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
	let size = static_cast<int8_t>(std::sqrt(m_grid.size()));
	let hs = int8_t(size / 2);
	let start = vector<int8_t>(module.grid_pos()) + vector<int8_t>(hs, hs);
	let end = vector<int8_t>(size, size) / int8_t(2);
	std::vector<Node> open = { Node{
		start, static_cast<int>(length(end - start)), nullptr } };
	std::vector<std::unique_ptr<Node>> closed;

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
			auto neighbour_pos = current->pos
				+ vector<int8_t>(i % 3 - 1, static_cast<int8_t>(i / 3 - 1));
			let traversable = neighbour_pos.x > 0 && neighbour_pos.y > 0
				&& neighbour_pos.x < size && neighbour_pos.y < size
				&& m_grid[neighbour_pos.y * size + neighbour_pos.x].first;
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

auto mark::unit::modular::p_at(vector<int8_t> user_pos) noexcept
	-> module::base*&
{
	return p_grid(to_new(user_pos)).first;
}

auto mark::unit::modular::p_at(vector<int8_t> user_pos) const noexcept
	-> const module::base*
{
	return p_grid(to_new(user_pos)).first;
}

auto mark::unit::modular::p_reserved(vector<int8_t> user_pos) noexcept -> bool&
{
	return p_grid(to_new(user_pos)).second;
}

auto mark::unit::modular::p_reserved(vector<int8_t> user_pos) const noexcept
	-> bool
{
	return p_grid(to_new(user_pos)).second;
}

void mark::unit::modular::ai(bool ai) { m_ai = ai; }

auto mark::unit::modular::radius() const -> double { return m_radius; }

auto mark::unit::modular::p_grid(vector<uint8_t> user_pos) noexcept
	-> std::pair<module::base*, bool>&
{
	return m_grid[user_pos.x + user_pos.y * max_size];
}

auto mark::unit::modular::p_grid(vector<uint8_t> user_pos) const noexcept
	-> const std::pair<module::base*, bool>&
{
	return m_grid[user_pos.x + user_pos.y * max_size];
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

void mark::unit::modular::tick(tick_context& context)
{
	let modifiers = this->modifiers();
	this->tick_modules(context);
	if (!m_ai && world().target().get() == this) {
		this->pick_up(context);
	}
	this->tick_movement([&] {
		tick_movement_info _;
		_.ai = m_ai;
		_.max_velocity = m_ai ? 64.0 : 320.0 + modifiers.velocity;
		_.acceleration = modifiers.mass > 1.f ? 5000.f / modifiers.mass : 500.f;
		_.dt = context.dt;
		return _;
	}());
	if (m_lookat != pos()) {
		let turn_speed = m_ai ? 32.f : 360.f;
		m_rotation = turn(m_lookat - pos(), m_rotation, turn_speed, context.dt);
	}
	if (m_ai) {
		for (let& command : this->tick_ai()) {
			this->command(command);
		}
	}
}

auto mark::unit::modular::neighbours_of(const module::base& module)
	-> std::vector<std::pair<std::reference_wrapper<module::base>, unsigned>>
{
	return ::neighbours_of<module::base>(
		*this,
		vector<int8_t>(module.grid_pos()),
		vector<int8_t>(module.size()));
}

auto mark::unit::modular::neighbours_of(const module::base& module) const
	-> std::vector<
		std::pair<std::reference_wrapper<const module::base>, unsigned>>
{
	return ::neighbours_of<const module::base>(
		*this,
		vector<int8_t>(module.grid_pos()),
		vector<int8_t>(module.size()));
}

auto mark::unit::modular::attach(
	const vector<int>& pos_,
	std::unique_ptr<interface::item>& item) -> std::error_code
{
	let module_pos = vector<int8_t>(pos_);
	if (!item || !can_attach(pos_, *item)) {
		return error::code::bad_pos;
	}
	std::unique_ptr<module::base> module(
		dynamic_cast<module::base*>(item.release()));
	return p_attach(pos_, module);
}

auto mark::unit::modular::p_attach(
	const vector<int>& pos_,
	std::unique_ptr<module::base>& module) -> std::error_code
{
	let module_pos = vector<int8_t>(pos_);
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
		this->p_at(module_pos + vector<int8_t>(i)) = module.get();
	}
	if (module->reserved() == module::reserved_kind::back) {
		for (let i : range<vector<int>>(
				 { -static_cast<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + static_cast<int>(module->size().y) })) {
			this->p_reserved(vector<int8_t>(i)) = true;
		}
	} else if (module->reserved() == module::reserved_kind::front) {
		for (let i : range<vector<int>>(
				 { module_pos.x + static_cast<int>(module->size().x),
				   module_pos.y },
				 { static_cast<int>(max_size / 2),
				   module_pos.y + static_cast<int>(module->size().y) })) {
			this->p_reserved(vector<int8_t>(i)) = true;
		}
	}
	m_radius = std::max(
		m_radius, length(module->pos() - this->pos())
		/* + length(module->size()) * static_cast<double>(module::size) / 2. */);
	m_modules.emplace_back(move(module));
	return error::code::success;
}

auto mark::unit::modular::can_attach(
	const vector<int>& pos_,
	const interface::item& item) const -> bool
{
	let module = dynamic_cast<const module::base*>(&item);
	if (!module) {
		return false;
	}
	return p_can_attach(*module, pos_)
		&& (m_modules.empty()
			|| !::neighbours_of<const module::base>(
					*this, vector<int8_t>(pos_), vector<int8_t>(module->size()))
					.empty());
}

auto mark::unit::modular::p_can_attach(
	const module::base& module,
	vector<int> pos_) const -> bool
{
	if (!(pos_.x >= -19 && pos_.x + static_cast<int>(module.size().x) < 19
		  && pos_.y >= -19
		  && pos_.y + static_cast<int>(module.size().y) < 19)) {
		return false;
	}
	let module_pos = vector<int8_t>(pos_);
	for (let i : range(vector<int8_t>(module.size()))) {
		if (this->p_at(module_pos + i) || this->p_reserved(module_pos + i)) {
			return false;
		}
	}
	// Establish core, check if core already present
	if (dynamic_cast<const module::core*>(&module) && m_core) {
		return false;
	}
	if (module.reserved() == module::reserved_kind::back) {
		for (let i : range<vector<int>>(
				 { -static_cast<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + static_cast<int>(module.size().y) })) {
			if (this->p_at(vector<int8_t>(i))) {
				return false;
			}
		}
	} else if (module.reserved() == module::reserved_kind::front) {
		for (let i : range<vector<int>>(
				 { module_pos.x + static_cast<int>(module.size().x),
				   module_pos.y },
				 { static_cast<int>(max_size / 2),
				   module_pos.y + static_cast<int>(module.size().y) })) {
			if (this->p_at(vector<int8_t>(i))) {
				return false;
			}
		}
	}
	return true;
}

auto mark::unit::modular::detach(const vector<int>& user_pos)
	-> std::unique_ptr<interface::item>
{

	let module_ptr = this->module_at(user_pos);
	if (!module_ptr) {
		return nullptr;
	}
	auto& module = *module_ptr;
	if (!module.detachable()) {
		return nullptr;
	}
	let module_pos = vector<int8_t>(module.grid_pos());
	let module_size = vector<int8_t>(module.size());
	// remove module from the grid
	let surface = range(module_pos, module_pos + module_size);
	for (let grid_pos : surface) {
		this->p_at(grid_pos) = nullptr;
	}
	let neighbours = this->neighbours_of(module);
	let disconnected =
		find_if(neighbours.begin(), neighbours.end(), [this](let& neighbour) {
			return !this->p_connected_to_core(neighbour.first.get());
		});
	if (disconnected != neighbours.end()) {
		for (let grid_pos : surface) {
			this->p_at(grid_pos) = module_ptr;
		}
		return nullptr;
	}
	if (module.reserved() == module::reserved_kind::back) {
		for (let i : range<vector<int>>(
				 { -static_cast<int>(max_size / 2), module_pos.y },
				 { module_pos.x,
				   module_pos.y + static_cast<int>(module.size().y) })) {
			this->p_reserved(vector<int8_t>(i)) = false;
		}
	} else if (module.reserved() == module::reserved_kind::front) {
		for (let i : range<vector<int>>(
				 { module_pos.x + module_size.x, module_pos.y },
				 { static_cast<int>(max_size / 2),
				   module_pos.y + static_cast<int>(module.size().y) })) {
			this->p_reserved(vector<int8_t>(i)) = false;
		}
	}
	this->unbind(module);
	let module_it = find_if(m_modules.begin(), m_modules.end(), [=](let& ptr) {
		return ptr.get() == module_ptr;
	});
	m_radius = 0.;
	for (let& cur_module : m_modules) {
		m_radius = std::max(
			m_radius, length(cur_module->pos() - this->pos())
			/* + length(module->size()) * static_cast<double>(module::size) / 2. */);
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

void mark::unit::modular::on_death(tick_context& context)
{
	for (auto& module : m_modules) {
		if (module->dead()) {
			continue;
		}
		context.units.emplace_back(std::make_shared<unit::bucket>([&] {
			unit::bucket::info info;
			info.world = &this->world();
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

auto mark::unit::modular::collide(const segment_t& ray) -> std::optional<
	std::pair<std::reference_wrapper<interface::damageable>, vector<double>>>
{
	std::optional<vector<double>> min;
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

auto mark::unit::modular::collide(vector<double> center, float radius)
	-> std::vector<std::reference_wrapper<interface::damageable>>
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
	std::vector<std::reference_wrapper<interface::damageable>> tmp;
	transform(out.cbegin(), out.cend(), back_inserter(tmp), [](let ptr) {
		return std::ref(*ptr);
	});
	return tmp;
}

auto mark::unit::modular::lookat() const noexcept -> vector<double>
{
	return m_lookat;
}

void mark::unit::modular::toggle_bind(int8_t command_id, vector<int> user_pos)
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

auto mark::unit::modular::binding(vector<int> user_pos) const
	-> std::vector<int8_t>
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
{
	std::unordered_map<uint64_t, std::reference_wrapper<module::base>> id_map;
	for (let& module_node : node["modules"]) {
		let module_pos = module_node["grid_pos"].as<vector<int>>();
		let id = module_node["id"].as<uint64_t>();
		auto item = [&] {
			let blueprint_node = module_node["blueprint"];
			auto& rm = world.resource_manager();
			if (!blueprint_node) {
				return module::deserialise(rm, module_node);
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
			return module::deserialise(rm, properties);
		}();
		std::unique_ptr<module::base> module(
			dynamic_cast<module::base*>(item.release()));
		assert(module);
		if (this->p_attach(module_pos, module) != error::code::success) {
			throw exception("BAD_MODULE_POS");
		}
		id_map.insert({ id, *m_modules.back() });
	}
	for (let& binding_node : node["bindings"]) {
		let key = static_cast<int8_t>(binding_node["key"].as<int>());
		let module_id = binding_node["module_id"].as<uint64_t>();
		m_bindings.insert({ key, id_map.at(module_id) });
	}
}

void mark::unit::modular::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	mobile::serialise(out);

	out << Key << "type" << Value << unit::modular::type_name;

	out << Key << "modules" << Value << BeginSeq;
	for (let& module : m_modules) {
		module->serialise(out);
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
		out << Key << "key" << Value << static_cast<int>(pair.first);
		out << Key << "module_id"
			<< reinterpret_cast<size_t>(&pair.second.get());
		out << EndMap;
	}
	out << EndSeq;

	// base
	out << EndMap;
}

auto mark::unit::modular::at(const vector<int>& module_pos) noexcept
	-> interface::item*
{
	return module_at(module_pos);
}

auto mark::unit::modular::at(const vector<int>& module_pos) const noexcept
	-> const interface::item*
{
	return module_at(module_pos);
}

auto mark::unit::modular::module_at(const vector<int>& module_pos) noexcept
	-> module::base*
{
	return const_cast<module::base*>(
		static_cast<const modular*>(this)->module_at(module_pos));
}

auto mark::unit::modular::module_at(const vector<int>& user_pos) const noexcept
	-> const module::base*
{
	let hs = static_cast<int8_t>(max_size / 2);
	if (user_pos.x >= -hs && user_pos.y < hs) {
		return this->p_at(vector<int8_t>(user_pos));
	}
	return nullptr;
}

auto mark::unit::modular::landed() const noexcept -> bool
{
	return !m_ai && world().target().get() != this;
}

void mark::unit::modular::remove_dead(tick_context& context)
{
	let first_dead_it = partition(
		m_modules.begin(),
		m_modules.end(),
		[](const std::unique_ptr<module::base>& module) {
			return !module->dead();
		});
	if (first_dead_it != m_modules.end()) {
		for_each(
			first_dead_it,
			m_modules.end(),
			[this, &context](std::unique_ptr<module::base>& module) {
				module->on_death(context);
				if (module.get() == m_core) {
					m_core = nullptr;
				}
				this->unbind(*module);
				let module_pos = vector<int8_t>(module->grid_pos());
				let module_size = vector<int8_t>(module->size());
				for (let i : range(module_size)) {
					this->p_at(module_pos + i) = nullptr;
				}
			});
		let first_detached_it =
			partition(m_modules.begin(), first_dead_it, [this](let& module) {
				return this->p_connected_to_core(*module);
			});
		for_each(first_detached_it, m_modules.end(), [this](let& module) {
			this->unbind(*module);
			let module_pos = vector<int8_t>(module->grid_pos());
			let module_size = vector<int8_t>(module->size());
			for (let i : range(module_size)) {
				this->p_at(module_pos + i) = nullptr;
			}
		});
		transform(
			make_move_iterator(first_detached_it),
			make_move_iterator(first_dead_it),
			back_inserter(context.units),
			[this](auto module) {
				unit::bucket::info info;
				info.world = &this->world();
				info.pos = this->pos();
				info.item = move(module);
				return std::make_shared<unit::bucket>(std::move(info));
			});
		m_modules.erase(first_detached_it, m_modules.end());
	}
}

void mark::unit::modular::pick_up(tick_context&)
{
	auto buckets = world().find<unit::bucket>(
		pos(), 150.f, [](let& unit) { return !unit.dead(); });
	auto containers = this->containers();
	for (auto& bucket : buckets) {
		auto module = bucket->release();
		for (auto& container : containers) {
			if (container.get().push(module) == error::code::success) {
				break;
			}
		}
		if (module) {
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

auto mark::unit::modular::containers()
	-> std::vector<std::reference_wrapper<module::cargo>>
{
	return filter_modules<module::cargo>(m_modules);
}

auto mark::unit::modular::containers() const
	-> std::vector<std::reference_wrapper<const module::cargo>>
{
	return filter_modules<const module::cargo>(m_modules);
}
