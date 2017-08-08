#include "stdafx.h"
#include "unit_modular.h"
#include "module_base.h"
#include "module_core.h"
#include "exception.h"
#include "sprite.h"
#include "world.h"
#include "command.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_landing_pad.h"
#include "module_shield_generator.h"
#include "module_cargo.h"
#include "unit_bucket.h"
#include "unit_gate.h"
#include "algorithm.h"

// MODULAR

namespace {
	// temporary refactor helper
	auto to_new(mark::vector<int8_t> pos) -> mark::vector<uint8_t> {
		const auto hs = static_cast<int8_t>(mark::unit::modular::max_size / 2);
		return {
			static_cast<uint8_t>(hs + pos.x),
			static_cast<uint8_t>(hs + pos.y)
		};
	}

	struct Node {
		mark::vector<int8_t> pos;
		int f = 0; // distance from starting + distance from ending (h)
		Node* parent = nullptr;
	};

	// Shared implementation of the modular::attached() function
	template <typename module_t, typename modular_t>
	static auto attached(modular_t& modular, const mark::module::base& module) {
		const auto size = mark::vector<int8_t>(module.size());
		const auto pos = mark::vector<int8_t>(module.grid_pos());

		std::vector<std::reference_wrapper<module_t>> out;
		auto out_insert = [&out](module_t* module_ptr) {
			if (module_ptr && (out.empty() || &out.back().get() != module_ptr)) {
				out.push_back(*module_ptr);
			}
		};
		// right
		for (const auto i : mark::enumerate(size.y)) {
			auto module_ptr = modular.at({ pos.x + size.x, pos.y + i });
			out_insert(module_ptr);
		}
		// bottom
		for (const auto i : mark::enumerate(size.x)) {
			auto module_ptr = modular.at({ pos.x + i, pos.y + size.y });
			out_insert(module_ptr);
		}
		// left
		for (const auto i : mark::enumerate(size.y)) {
			auto module_ptr = modular.at({ pos.x - 1, pos.y + i });
			out_insert(module_ptr);
		}
		// top
		for (const auto i : mark::enumerate(size.x)) {
			auto module_ptr = modular.at({ pos.x + i, pos.y - 1 });
			out_insert(module_ptr);
		}
		return out;
	}
}



mark::unit::modular::modular(
	mark::world& world,
	mark::vector<double> pos,
	float rotation):
	mark::unit::base(world, pos),
	m_rotation(rotation) {}

void mark::unit::modular::tick_modules(mark::tick_context& context) {
	this->remove_dead(context);
	for (auto& module : m_modules) {
		// Module might be already dead, don't tick dead modules
		if (!module->dead()) {
			module->tick(context);
		}
	}
}

void mark::unit::modular::tick_movement(
	double dt,
	const mark::module::modifiers& mods) {

	double speed = ((m_ai ? 64.0 : 320.0) + mods.velocity) + mods.velocity;
	const auto radius = 75.f;
	if (mark::length(m_moveto - pos()) > speed * dt) {
		if (team() == 1
			|| (m_path_age <= 0.f || m_path_cache.size() > 0 && mark::length(m_path_cache.back() - m_moveto) < radius)
				&& m_world.map().can_find()) {
			m_path_cache = m_world.map().find_path(pos(), m_moveto, radius);
			m_path_age = 1.f;
		} else {
			m_path_age -= static_cast<float>(dt);
		}
		const auto dir = mark::normalize(m_moveto - pos());

		if (m_path_cache.size() > 3) {
			const auto first = m_path_cache[m_path_cache.size() - 3];
			pos() += mark::normalize(first - pos()) * speed * dt;
		} else {
			const auto step = mark::normalize(m_moveto - pos()) * speed * dt;
			// TODO - intersect radius with terrain, stick to the wall, follow x/y axis
			if (m_world.map().traversable(pos() + step, radius)) {
				pos() += step;
			} else if (m_world.map().traversable(pos() + mark::vector<double>(step.x, 0), radius)) {
				pos() += mark::vector<double>(step.x, 0);
			} else if (m_world.map().traversable(pos() + mark::vector<double>(0, step.y), radius)) {
				pos() += mark::vector<double>(0, step.y);
			}
		}
	} else {
		pos() = m_moveto;
	}
	if (m_lookat != pos()) {
		const auto turn_speed = m_ai ? 32.f : 360.f;
		m_rotation = mark::turn(m_lookat - pos(), m_rotation, turn_speed, dt);
	}
}

void mark::unit::modular::tick_ai() {
	auto enemy = m_world.find_one(
		pos(),
		1000.f,
		[this](const auto& unit) {
		return unit.team() != this->team() && !unit.invincible();
	});
	if (enemy) {
		m_moveto = enemy->pos();
		m_lookat = enemy->pos();
		for (auto& module : m_modules) {
			module->target(enemy->pos());
		}
	}
}

auto mark::unit::modular::p_connected_to_core(const mark::module::base& module) const -> bool {
	const auto size = static_cast<int8_t>(std::sqrt(m_grid.size()));
	const auto hs = int8_t(size / 2);
	const auto start = mark::vector<int8_t>(module.grid_pos()) + mark::vector<int8_t>(hs, hs);
	const auto end = mark::vector<int8_t>(size, size) / int8_t(2);
	std::vector<Node> open = { Node{ start, static_cast<int>(mark::length(end - start)), nullptr } };
	std::vector<std::unique_ptr<Node>> closed;

	while (!open.empty()) {
		// TODO use std::min to find the smallest it
		auto min_it = open.begin();
		for (auto it = open.begin(), end = open.end(); it != end; it++) {
			if (min_it->f > it->f) {
				min_it = it;
			}
		}
		closed.push_back(std::make_unique<Node>(*min_it));
		auto& current = closed.back();
		// TODO replace with the last element and pop back, instead of erase
		open.erase(min_it);

		if (current->pos == end) {
			return true;
		}

		// TODO Replace with int8_t, and remove vector casts
		// TODO Replace with enumerate
		for (int i = 1; i < 8; i += 2) {
			auto neighbour_pos = current->pos + mark::vector<int8_t>(i % 3 - 1, static_cast<int8_t>(i / 3 - 1));
			const auto traversable = neighbour_pos.x > 0
				&& neighbour_pos.y > 0 && neighbour_pos.x < size
				&& neighbour_pos.y < size
				&& m_grid[neighbour_pos.y * size + neighbour_pos.x];
			const auto isClosed = closed.end() != std::find_if(
				closed.begin(),
				closed.end(),
				[&neighbour_pos](const auto& node) {
				return node->pos == neighbour_pos;
			});
			if (!traversable || isClosed) {
				continue;
			}
			auto neighbour_it = std::find_if(
				open.begin(),
				open.end(),
				[&neighbour_pos](const auto& node) {
				return neighbour_pos == node.pos;
			});
			const auto f = current->f + 10;
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

auto mark::unit::modular::p_at(mark::vector<int8_t> pos) noexcept ->
	mark::module::base*& {
	return p_grid(to_new(pos));
}

auto mark::unit::modular::p_at(mark::vector<int8_t> pos) const noexcept ->
	const mark::module::base* {
	return p_grid(to_new(pos));
}

auto mark::unit::modular::p_grid(mark::vector<uint8_t> pos) noexcept ->
	mark::module::base*& {
	return m_grid[pos.x + pos.y * max_size];
}

auto mark::unit::modular::p_grid(mark::vector<uint8_t> pos) const noexcept ->
	const mark::module::base* {
	return m_grid[pos.x + pos.y * max_size];
}


auto mark::unit::modular::modifiers() const -> mark::module::modifiers {
	mark::module::modifiers mods;
	for (auto& module : m_modules) {
		const auto cur_mod = module->global_modifiers();
		mods.velocity += cur_mod.velocity;
	}
	return mods;
}

void mark::unit::modular::tick(mark::tick_context& context) {
	const auto modifiers = this->modifiers();
	this->tick_modules(context);
	if (!m_ai) {
		this->pick_up(context);
	}
	this->tick_movement(context.dt, modifiers);
	if (m_ai) {
		this->tick_ai();
	}
}

auto mark::unit::modular::get_attached(const mark::module::base& module) ->
std::vector<std::reference_wrapper<mark::module::base>> {
	return ::attached<mark::module::base>(*this, module);
}

auto mark::unit::modular::attached(const mark::module::base& module) ->
std::vector<std::reference_wrapper<mark::module::base>> {
	return ::attached<mark::module::base>(*this, module);
}

auto mark::unit::modular::attached(const mark::module::base& module) const ->
std::vector<std::reference_wrapper<const mark::module::base>> {
	return ::attached<const mark::module::base>(*this, module);
}

void mark::unit::modular::attach(
	std::unique_ptr<mark::module::base>& module,
	mark::vector<int> pos_) {
	const auto module_pos = mark::vector<int8_t>(pos_);
	if (!module) {
		throw mark::exception("NULL_MODULE");
	}
	for (const auto i : mark::enumerate(mark::vector<int8_t>(module->size()))) {
		if (this->p_at(module_pos + i)) {
			throw mark::user_error("MODULE_OVERLAP");
		}
	}
	// establish core, check if core already present
	auto core = dynamic_cast<mark::module::core*>(module.get());
	if (core) {
		if (m_core) {
			throw mark::user_error("TWO_CORES");
		} else {
			m_core = core;
		}
	}
	module->m_grid_pos = module_pos;
	module->m_parent = this;
	// check if has neighbours
	if (!core && module->neighbours().empty()) {
		module->m_parent = nullptr;
		throw mark::user_error("NO_NEIGHBOURS");
	}
	for (const auto i : mark::enumerate(module->size())) {
		this->p_at(module_pos + mark::vector<int8_t>(i)) = module.get();
	}
	m_modules.emplace_back(std::move(module));
}

void mark::unit::modular::p_attach(
	std::unique_ptr<mark::module::base> module,
	mark::vector<int> pos_) {
	const auto module_pos = mark::vector<int8_t>(pos_);
	if (!module) {
		throw mark::exception("NULL_MODULE");
	}
	for (const auto i : mark::enumerate(mark::vector<int8_t>(module->size()))) {
		if (this->p_at(module_pos + i)) {
			throw mark::user_error("MODULE_OVERLAP");
		}
	}
	// establish core, check if core already present
	auto core = dynamic_cast<mark::module::core*>(module.get());
	if (core) {
		if (m_core) {
			throw mark::user_error("TWO_CORES");
		} else {
			m_core = core;
		}
	}
	module->m_grid_pos = module_pos;
	module->m_parent = this;
	for (const auto i : mark::enumerate(module->size())) {
		this->p_at(module_pos + mark::vector<int8_t>(i)) = module.get();
	}
	m_modules.emplace_back(std::move(module));
}

auto mark::unit::modular::can_attach(
	const std::unique_ptr<module::base>& module,
	mark::vector<int> pos) const -> bool {
	// TODO deprecated
	return true;
}

auto mark::unit::modular::module(mark::vector<int> pos) const ->
	const mark::module::base* {
	return this->at(mark::vector<int8_t>(pos));
}

auto mark::unit::modular::module(mark::vector<int> pos)->mark::module::base* {
	const auto cthis = static_cast<const mark::unit::modular*>(this);
	return this->at(mark::vector<int8_t>(pos));
}

// vector erase, don't preserve element order
template<typename vector_t, typename iterator_t>
auto drop(vector_t& vector, iterator_t it) {
	auto owner = std::move(*it);
	std::swap(*it, vector.back());
	vector.pop_back();
	return std::move(owner);
}

auto mark::unit::modular::detach(mark::vector<int> pos_) ->
	std::unique_ptr<mark::module::base> {
	const auto pos = mark::vector<int8_t>(pos_);

	// Remove module from grid
	if (const auto module_ptr = this->at(pos)) {
		auto& module = *module_ptr;
		const auto module_pos = vector<int8_t>(module.grid_pos());
		const auto module_size = mark::vector<int8_t>(module.size());
		for (const auto i : enumerate(module_size)) {
			this->p_at(module_pos + i) = nullptr;
		}
		try {
			// Check if module is essential
			for (const auto neighbour : module.neighbours()) {
				if (!this->p_connected_to_core(neighbour)) {
					throw mark::exception("MODULE_ESSENTIAL");
				}
			}
			this->unbind(module);
			const auto module_it = std::find_if(
				m_modules.begin(), m_modules.end(), [=](const auto& ptr) {
				return ptr.get() == module_ptr;
			});
			return std::move(drop(m_modules, module_it));
		} catch (const std::exception&) {
			// Roll back in case of bad alloc or module being essential
			for (const auto i : enumerate(module_size)) {
				this->p_at(module_pos + i) = module_ptr;
			}
		}
	}
	return nullptr;
}

auto mark::unit::modular::get_core() -> mark::module::core& {
	if (m_core) {
		return *m_core;
	} else {
		throw mark::user_error("NO_CORE");
	}
}

auto mark::unit::modular::core() -> mark::module::core& {
	if (m_core) {
		return *m_core;
	} else {
		throw mark::user_error("NO_CORE");
	}
}

auto mark::unit::modular::core() const -> const mark::module::core& {
	if (m_core) {
		return *m_core;
	} else {
		throw mark::user_error("NO_CORE");
	}
}

void mark::unit::modular::command(const mark::command& command) {
	if (command.type == mark::command::type::move) {
		m_moveto = command.pos;
		m_move = !command.release;
	} else if (command.type == mark::command::type::guide) {
		m_lookat = command.pos;
		for (auto& module : m_modules) {
			module->target(command.pos);
		}
		if (m_move) {
			m_moveto = m_lookat;
		}
	} else if (command.type == mark::command::type::ai) {
		m_ai = true;
	} else if (command.type == mark::command::type::activate && !command.release) {
		auto pad = m_world.find_one(
			pos(),
			150.0,
			[this](const auto& unit) {
			return dynamic_cast<const mark::unit::landing_pad*>(&unit) != nullptr
				|| dynamic_cast<const mark::unit::gate*>(&unit) != nullptr;
		});
		if (pad) {
			pad->activate(this->shared_from_this());
		}
	} else {
		auto bindings = m_bindings.equal_range(command.type);
		std::for_each(bindings.first, bindings.second, [&command](auto module) {
			module.second.get().shoot(command.pos, command.release);
		});
	}
}

auto mark::unit::modular::dead() const -> bool {
	return !m_core || m_core->dead();
}

void mark::unit::modular::on_death(mark::tick_context& context) {
	for (auto& module : m_modules) {
		if (!module->dead()) {
			context.units.emplace_back(std::make_shared<mark::unit::bucket>(
				m_world,
				pos(),
				std::move(module)));
		}
	}
}

bool mark::unit::modular::damage(const mark::idamageable::info& attr) {
	for (auto& module : m_modules) {
		if (module->damage(attr)) {
			attr.damaged->insert(this);
			return true;
		}
	}
	return false;
}

auto mark::unit::modular::collide(const mark::segment_t& ray) ->
std::pair<mark::idamageable *, mark::vector<double>> {
	auto min = mark::vector<double>(NAN, NAN);
	double min_length = INFINITY;
	mark::idamageable* damageable = nullptr;
	for (auto& module : m_modules) {
		auto result = module->collide(ray);
		if (result.first) {
			const auto length = mark::length(ray.first - result.second);
			if (length < min_length) {
				min_length = length;
				min = result.second;
				damageable = result.first;
			}

		}
	}
	// check if module is under the shield
	std::vector<std::reference_wrapper<mark::module::shield_generator>> shields;
	for (auto& module : m_modules) {
		const auto shield = dynamic_cast<mark::module::shield_generator*>(module.get());
		if (shield && shield->shield() > 0.f) {
			shields.push_back(*shield);
		}
	}
	for (auto shield : shields) {
		const auto shield_pos = shield.get().pos();
		const auto shield_size = 64.f;
		if (mark::length(min - shield_pos) < shield_size - 1.f) {
			return { nullptr, { NAN, NAN } };
		}
	}
	return { damageable, min };
}

auto mark::unit::modular::collide(mark::vector<double> center, float radius) ->
std::vector<std::reference_wrapper<mark::idamageable>> {
	std::unordered_set<mark::idamageable*> out;
	// get shields
	std::vector<std::reference_wrapper<mark::module::shield_generator>> shields;
	for (auto& module : m_modules) {
		const auto shield = dynamic_cast<mark::module::shield_generator*>(module.get());
		if (shield && shield->shield() >= 0.f) {
			shields.push_back(*shield);
		}
	}
	for (auto& module : m_modules) {
		const auto module_size = std::max(module->size().x, module->size().y);
		const auto module_pos = module->pos();
		if (mark::length(center - module_pos) < module_size + radius) {
			for (auto shield : shields) {
				const auto shield_pos = shield.get().pos();
				const auto shield_size = 64.f;
				if (mark::length(module_pos - shield_pos) < shield_size) {
					out.insert(&shield.get());
					goto outer_continue;
				}
			}
			out.insert(module.get());
		}
	outer_continue:;
	}
	std::vector<std::reference_wrapper<mark::idamageable>> tmp;
	std::transform(
		out.begin(),
		out.end(),
		std::back_inserter(tmp),
		[](auto module) { return std::ref(*module); }
	);
	return tmp;
}

auto mark::unit::modular::lookat() const noexcept -> mark::vector<double> {
	return m_lookat;
}

void mark::unit::modular::toggle_bind(enum class mark::command::type command, mark::vector<int> pos) {
	if (const auto module_ptr = this->at(mark::vector<int8_t>(pos))) {
		auto& module = *module_ptr;
		const auto bindings = m_bindings.equal_range(command);
		const auto binding = std::find_if(
			bindings.first,
			bindings.second,
			[&module](auto pair) {
				return &module == &pair.second.get();
			}
		);
		if (binding == bindings.second) {
			m_bindings.insert({ command, module });
		} else {
			m_bindings.erase(binding);
		}
	}
}

auto mark::unit::modular::bound_status() const ->
	std::array<struct mark::unit::modular::bound_status, 11> {
	std::array<struct mark::unit::modular::bound_status, 11> out;
	for (const auto& binding : m_bindings) {
		uint8_t slot;
		if (binding.first == mark::command::type::shoot) {
			slot = 0;
		} else if (binding.first == mark::command::type::ability_0) {
			slot = 10;
		} else {
			slot = static_cast<uint8_t>(binding.first) -
				static_cast<uint8_t>(mark::command::type::ability_1) + 1;
		}
		out[slot].total++;
		out[slot].thumbnail = binding.second.get().thumbnail();
	}
	return out;
}

// Serializer / Deserializer

mark::unit::modular::modular(mark::world& world, const YAML::Node& node):
	mark::unit::base(world, node),
	m_moveto(node["moveto"].as<mark::vector<double>>()),
	m_lookat(node["lookat"].as<mark::vector<double>>()),
	m_ai(node["ai"].as<bool>()),
	m_move(node["move"].as<bool>()) {

	std::unordered_map<uint64_t, std::reference_wrapper<mark::module::base>> id_map;
	for (const auto& module_node : node["modules"]) {
		const auto pos = module_node["grid_pos"].as<mark::vector<int>>();
		const auto id = module_node["id"].as<uint64_t>();
		auto module = mark::module::deserialize(world.resource_manager(), module_node);
		this->p_attach(std::move(module), pos);
		id_map.insert({ id, *m_modules.back() });
	}
	for (const auto& binding_node : node["bindings"]) {
		const auto key = binding_node["key"].as<int>();
		const auto module_id = binding_node["module_id"].as<uint64_t>();
		m_bindings.insert({
			static_cast<enum class mark::command::type>(key),
			id_map.at(module_id)
		});
	}
}

void mark::unit::modular::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	this->serialize_base(out);

	out << Key << "type" << Value << mark::unit::modular::type_name;

	out << Key << "modules" << Value << BeginSeq;
	for (const auto& module : m_modules) {
		module->serialize(out);
	}
	out << EndSeq;

	out << Key << "moveto" << Value << BeginMap;
	out << Key << "x" << Value << m_moveto.x;
	out << Key << "y" << Value << m_moveto.y;
	out << EndMap;

	out << Key << "lookat" << Value << BeginMap;
	out << Key << "x" << Value << m_lookat.x;
	out << Key << "y" << Value << m_lookat.y;
	out << EndMap;

	out << Key << "ai" << Value << m_ai;

	out << Key << "move" << Value << m_move;

	out << Key << "bindings" << Value << BeginSeq;

	for (const auto& pair : m_bindings) {
		out << BeginMap;
		out << Key << "key" << Value << static_cast<int>(pair.first);
		out << Key << "module_id" << pair.second.get().id();
		out << EndMap;
	}
	out << EndSeq;

	// base
	out << EndMap;
}

auto mark::unit::modular::at(mark::vector<int8_t> pos) noexcept ->
	mark::module::base* {
	return const_cast<module::base*>(static_cast<const modular*>(this)->at(pos));
}

auto mark::unit::modular::at(mark::vector<int8_t> pos) const noexcept ->
	const mark::module::base* {
	const auto hs = static_cast<int8_t>(max_size / 2);
	if (pos.x >= -hs && pos.y < hs) {
		return this->p_at(pos);
	} else {
		return nullptr;
	}
}

void mark::unit::modular::remove_dead(mark::tick_context& context) {
	const auto first_dead_it = std::partition(
		m_modules.begin(),
		m_modules.end(),
		[](const std::unique_ptr<mark::module::base>& module) {
			return !module->dead();
	});
	if (first_dead_it != m_modules.end()) {
		for (const auto& module : m_modules) {
			if (module.get() == m_core) {
				m_core = nullptr;
			}
			module->on_death(context);
			this->unbind(*module);
			const auto module_pos = mark::vector<int8_t>(module->grid_pos());
			const auto module_size = mark::vector<int8_t>(module->size());
			for (const auto i : mark::enumerate(module_size)) {
				this->p_at(module_pos + i) = nullptr;
			}
		}
		const auto first_detached_it = std::partition(
			m_modules.begin(),
			m_modules.end(),
			[this](const auto& module) {
				return this->p_connected_to_core(*module);
		});
		std::for_each(
			first_detached_it,
			m_modules.end(),
			[this](const auto& module) {
			this->unbind(*module);
			const auto module_pos = mark::vector<int8_t>(module->grid_pos());
			const auto module_size = mark::vector<int8_t>(module->size());
			for (const auto i : mark::enumerate(module_size)) {
				this->p_at(module_pos + i) = nullptr;
			}
		});
		std::transform(
			std::make_move_iterator(first_detached_it),
			std::make_move_iterator(m_modules.end()),
			std::back_inserter(context.units),
			[this](auto module) {
			return std::make_shared<mark::unit::bucket>(
				m_world, this->pos(), std::move(module));
		});
		m_modules.erase(first_detached_it, m_modules.end());
	}
}

void mark::unit::modular::pick_up(mark::tick_context& context) {
	auto units = m_world.find(pos(), 150.f, [](const auto& unit) {
		return !unit.dead();
	});
	auto containers = this->containers();
	for (auto& unit : units) {
		const auto bucket = std::dynamic_pointer_cast<mark::unit::bucket>(unit);
		if (bucket) {
			auto module = bucket->release();
			for (auto& container : containers) {
				if (container.get().push(module)) {
					break;
				}
			}
			if (module) {
				bucket->insert(std::move(module));
			}
		}
	}
}

void mark::unit::modular::unbind(const mark::module::base& module) {
	auto it = m_bindings.begin();
	const auto end = m_bindings.end();
	while (it != end) {
		auto cur = it;
		it++;
		if (&cur->second.get() == &module) {
			m_bindings.erase(cur);
		}
	}
}

auto mark::unit::modular::invincible() const -> bool {
	return false;
}

void mark::unit::modular::activate(const std::shared_ptr<mark::unit::base>& by) {
	m_world.target(this->shared_from_this());
}

auto mark::unit::modular::containers() ->
	std::vector<std::reference_wrapper<mark::module::cargo>> {
	std::vector<std::reference_wrapper<mark::module::cargo>> out;
	for (auto& module : m_modules) {
		auto cargo = dynamic_cast<mark::module::cargo*>(module.get());
		if (cargo) {
			out.push_back(*cargo);
		}
	}
	return out;
}
