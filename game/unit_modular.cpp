#include "stdafx.h"
#include "unit_modular.h"
#include "module_base.h"
#include "module_core.h"
#include "exception.h"
#include "sprite.h"
#include "world.h"
#include "terrain_base.h"
#include "command.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_landing_pad.h"
#include "module_shield_generator.h"
#include "module_cargo.h"
#include "unit_bucket.h"
#include "unit_gate.h"

// MODULAR

namespace {
	// point in a box
	bool overlap(
		mark::vector<int> pos1,
		mark::vector<unsigned> size1,
		mark::vector<int> pos2) {
		const auto br1 = pos1 + mark::vector<int>(size1);
		return pos1.x <= pos2.x && pos1.y <= pos2.y
			&& pos2.x < br1.x && pos2.y < br1.y;
	}
	// two rectangles overlap
	bool overlap(
		mark::vector<int> pos1,
		mark::vector<unsigned> size1,
		mark::vector<int> pos2,
		mark::vector<unsigned> size2) {
		const auto br1 = pos1 + mark::vector<int>(size1);
		const auto br2 = pos2 + mark::vector<int>(size2);
		return pos1.x < br2.x && pos1.y < br2.y
			&& pos2.x < br1.x && pos2.y < br1.y;
	}
	// map of booleans 1 - occupied by a module, 0 - available
	static auto availability_map(const std::vector<std::unique_ptr<mark::module::base>>& modules) {
		const auto max_size = mark::unit::modular::max_size;
		std::vector<bool> map(max_size * max_size, false);
		const auto hs = mark::vector<int8_t>(max_size / 2, max_size / 2);
		for (const auto& module : modules) {
			for (int8_t x = 0; x < static_cast<int8_t>(module->size().x); x++) {
				for (int8_t y = 0; y < static_cast<int8_t>(module->size().y); y++) {
					const auto px = hs.x + module->grid_pos().x + x;
					const auto py = hs.y + module->grid_pos().y + y;
					map[py * max_size + px] = true;
				}
			}
		}
		return map;
	}

	struct Node {
		mark::vector<int8_t> pos;
		int f = 0; // distance from starting + distance from ending (h)
		Node* parent = nullptr;
	};

	auto path_exists(const std::vector<bool>& map, mark::vector<int8_t> start) -> bool {
		const auto size = static_cast<int8_t>(std::sqrt(map.size()));
		const auto end_x = size / static_cast<int8_t>(2);
		const auto end = mark::vector<int8_t>(end_x, end_x);
		std::vector<Node> open = { Node{ start, static_cast<int>(mark::length(end - start)), nullptr } };
		std::vector<std::unique_ptr<Node>> closed;

		while (!open.empty()) {
			auto min_it = open.begin();
			for (auto it = open.begin(), end = open.end(); it != end; it++) {
				if (min_it->f > it->f) {
					min_it = it;
				}
			}
			closed.push_back(std::make_unique<Node>(*min_it));
			auto& current = closed.back();
			open.erase(min_it);

			if (current->pos == end) {
				return true;
			}

			for (int i = 1; i < 8; i += 2) {
				auto neighbour_pos = current->pos + mark::vector<int8_t>(i % 3 - 1, static_cast<int8_t>(i / 3 - 1));
				const auto traversable = neighbour_pos.x > 0
					&& neighbour_pos.y > 0 && neighbour_pos.x < size
					&& neighbour_pos.y < size
					&& map[neighbour_pos.y * size + neighbour_pos.x];
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

	template <typename object_t, typename modules_t>
	static auto attached(
		modules_t& modules,
		mark::vector<int8_t> pos,
		mark::vector<uint8_t> size) {

		std::vector<mark::vector<int>> border;
		// right
		for (int i = 0; i < size.y; i++) {
			border.emplace_back(pos.x + size.x + 1, pos.y + i);
		}
		// bottom
		for (int i = 0; i < size.x; i++) {
			border.emplace_back(pos.x + i, pos.y + size.y + 1);
		}
		// left
		for (int i = 0; i < size.y; i++) {
			border.emplace_back(pos.x - 1, pos.y + i);
		}
		// top
		for (int i = 0; i < size.x; i++) {
			border.emplace_back(pos.x + i, pos.y - 1);
		}
		std::vector<std::reference_wrapper<object_t>> out;
		for (auto& module : modules) {
			for (auto& pos : border) {
				if (::overlap(module->grid_pos(), module->size(), pos)) {
					out.push_back(*module);
					break;
				}
			}
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
		module->tick(context);
	}
}

void mark::unit::modular::tick_movement(
	double dt,
	const mark::module::modifiers& mods) {

	double speed = m_ai ? 64.0 : 320.0;
	speed += mods.velocity;
	if (mark::length(m_moveto - m_pos) > speed * dt) {
		if ((m_path_age <= 0.f || m_path.size() > 0 && mark::length(m_path.back() - m_moveto) < 150.f) && m_world.map().can_find()) {
			m_path = m_world.map().find_path(m_pos, m_moveto);
			m_path_age = 1.f;
		} else {
			m_path_age -= static_cast<float>(dt);
		}
		const auto dir = mark::normalize(m_moveto - m_pos);
		if (m_path.size() > 3) {
			const auto first = m_path[m_path.size() - 3];
			m_pos += mark::normalize(first - m_pos) * speed * dt;
		} else {
			const auto step = mark::normalize(m_moveto - m_pos) * speed * dt;
			if (m_world.map().traversable(m_pos + step, 50.0)) {
				m_pos += step;
			}
		}
	} else {
		m_pos = m_moveto;
	}
	if (m_lookat != m_pos) {
		const auto turn_speed = m_ai ? 32.f : 360.f;
		m_rotation = mark::turn(m_lookat - m_pos, m_rotation, turn_speed, dt);
	}
}

void mark::unit::modular::tick_ai() {
	auto enemy = m_world.find_one(
		m_pos,
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

auto mark::unit::modular::modifiers() const -> mark::module::modifiers {
	mark::module::modifiers mods;
	for (auto& module : m_modules) {
		const auto cur_mod = module->global_modifiers();
		mods.velocity += cur_mod.velocity;
	}
	return mods;
}

void mark::unit::modular::tick(mark::tick_context& context) {
	if (this->dead()) {
		return;
	}
	const auto modifiers = this->modifiers();
	this->tick_modules(context);
	this->pick_up(context);
	this->tick_movement(context.dt, modifiers);
	if (m_ai) {
		this->tick_ai();
	}
}

auto mark::unit::modular::get_attached(mark::module::base& module) ->
std::vector<std::reference_wrapper<mark::module::base>> {
	return ::attached<mark::module::base>(
		m_modules,
		mark::vector<int8_t>(module.grid_pos()),
		mark::vector<uint8_t>(module.size()));
}

auto mark::unit::modular::attached(
	mark::vector<int8_t> pos,
	mark::vector<uint8_t> size) const ->
	std::vector<std::reference_wrapper<const mark::module::base>> {
	
	return ::attached<const mark::module::base>(m_modules, pos, size);
}

void mark::unit::modular::attach(
	std::unique_ptr<mark::module::base> module,
	mark::vector<int> pos) {

	if (!module) {
		throw mark::exception("NULL_MODULE");
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
	module->m_grid_pos = mark::vector<int8_t>(pos);
	module->m_parent = this;
	m_modules.emplace_back(std::move(module));
}

auto mark::unit::modular::can_attach(
	const std::unique_ptr<module::base>& module,
	mark::vector<int> pos) const -> bool {

	if (!module) {
		return false;
	}
	const auto size = module->size();
	for (auto& module : m_modules) {
		if (::overlap(mark::vector<int>(module->grid_pos()), module->size(), pos, size)) {
			return false;
		}
	}
	if (this->attached(mark::vector<int8_t>(pos), mark::vector<uint8_t>(module->size())).empty()) {
		return false;
	}
	return true;
}

auto mark::unit::modular::module(mark::vector<int> pos) const ->
	const mark::module::base*{

	for (auto& module : m_modules) {
		if (::overlap(module->grid_pos(), module->size(), pos)) {
			return module.get();
		}
	}
	return nullptr;
}

auto mark::unit::modular::module(mark::vector<int> pos)->mark::module::base* {
	const auto cthis = static_cast<const mark::unit::modular*>(this);
	return const_cast<mark::module::base*>(cthis->module(pos));
}

auto mark::unit::modular::detach(mark::vector<int> pos) ->
	std::unique_ptr<mark::module::base> {

	auto module_it = std::find_if(
		m_modules.begin(),
		m_modules.end(),
		[&pos](const auto& module) {
		return overlap(module->grid_pos(), module->size(), pos);
	});
	if (module_it != m_modules.end() && (*module_it)->detachable()) {
		const auto& module = *module_it;
		// check if module is essential

		auto map = availability_map(m_modules);
		const auto hs = mark::vector<int8_t>(max_size / 2, max_size / 2);
		for (int8_t x = 0; x < static_cast<int8_t>(module->size().x); x++) {
			for (int8_t y = 0; y < static_cast<int8_t>(module->size().y); y++) {
				const auto px = hs.x + module->grid_pos().x + x;
				const auto py = hs.y + module->grid_pos().y + y;
				map[py * max_size + px] = false;
			}
		}
		const auto neighbours = (*module_it)->neighbours();
		for (const auto& neighbour : neighbours) {
			const auto grid_pos = hs + mark::vector<int8_t>(neighbour.get().grid_pos());
			if (map[grid_pos.y * mark::unit::modular::max_size + grid_pos.x]
				&& !path_exists(map, grid_pos)) {
				return nullptr;
			}
		}

		// detach
		auto out = std::move(*module_it);
		if (module_it != std::prev(m_modules.end())) {
			*module_it = std::move(m_modules.back());
		}
		m_modules.pop_back();
		this->unbind(*out);
		return std::move(out);
	} else {
		return nullptr;
	}
}

auto mark::unit::modular::get_core() -> mark::module::core& {
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
			m_pos,
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
	return m_core && m_core->dead();
}

void mark::unit::modular::on_death(mark::tick_context& context) {
	for (auto& module : m_modules) {
		if (!module->dead()) {
			context.units.emplace_back(std::make_shared<mark::unit::bucket>(
				m_world,
				m_pos,
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
	const auto module_it = std::find_if(
		m_modules.begin(),
		m_modules.end(),
		[pos](const auto& module) {
		return ::overlap(mark::vector<int>(module->grid_pos()), module->size(), pos);
	});
	if (module_it != m_modules.end()) {
		auto& module = **module_it;
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
	m_moveto(node["moveto"]["x"].as<double>(), node["moveto"]["y"].as<double>()),
	m_lookat(node["lookat"]["x"].as<double>(), node["lookat"]["y"].as<double>()),
	m_ai(node["ai"].as<bool>()),
	m_move(node["move"].as<bool>()) {

	std::unordered_map<uint64_t, std::reference_wrapper<mark::module::base>> id_map;
	for (const auto& module_node : node["modules"]) {
		const auto pos_x = module_node["grid_pos"]["x"].as<int>();
		const auto pos_y = module_node["grid_pos"]["y"].as<int>();
		const auto pos = mark::vector<int>(pos_x, pos_y);
		const auto id = module_node["id"].as<uint64_t>();
		auto module = mark::module::deserialize(world.resource_manager(), module_node);
		this->attach(std::move(module), pos);
		id_map.insert({ id, *m_modules.back() });
	}
	for (const auto& binding_node : node["bindings"]) {
		const auto key = binding_node["key"].as<int>();
		const auto module_id = binding_node["module_id"].as<uint64_t>();
		m_bindings.insert({ static_cast<enum class mark::command::type>(key), id_map.at(module_id) });
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

void mark::unit::modular::remove_dead(mark::tick_context& context) {
	auto end_it = std::partition(
		m_modules.begin(),
		m_modules.end(),
		[](const std::unique_ptr<mark::module::base>& module) {
			return !module->dead();
	});
	if (end_it != m_modules.end()) {
		std::for_each(
			end_it,
			m_modules.end(),
			[&context, this](auto& module) {
			module->on_death(context);
			this->unbind(*module);
		});
		m_modules.erase(end_it, m_modules.end());
		{
			auto map = availability_map(m_modules);
			auto end_it = std::partition(
				m_modules.begin(),
				m_modules.end(),
				[&map](const auto& module) {
					const auto max_size = mark::unit::modular::max_size;
					const auto hs = mark::vector<int8_t>(max_size / 2, max_size / 2);
					return ::path_exists(map, hs + mark::vector<int8_t>(module->grid_pos()));
				}
			);
			std::transform(
				std::make_move_iterator(end_it),
				std::make_move_iterator(m_modules.end()),
				std::back_inserter(context.units),
				[this](auto module) {
				return std::make_shared<mark::unit::bucket>(m_world, m_pos, std::move(module));
			});
			m_modules.erase(end_it, m_modules.end());
		}
	}
}

void mark::unit::modular::pick_up(mark::tick_context& context) {
	auto buckets = m_world.find(m_pos, 150.f, [](const auto& unit) {
		return dynamic_cast<const mark::unit::bucket*>(&unit) != nullptr && !unit.dead();
	});
	auto containers = this->containers();
	for (auto& bucket : buckets) {
		auto module = std::dynamic_pointer_cast<mark::unit::bucket>(bucket)->release();
		for (auto& container : containers) {
			if (container.get().push(module)) {
				break;
			}
		}
		if (module) {
			context.units.push_back(std::make_shared<mark::unit::bucket>(m_world, bucket->pos(), std::move(module)));
			break;
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
