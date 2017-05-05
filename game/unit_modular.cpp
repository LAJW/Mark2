#include <assert.h>
#include <cmath>
#include <algorithm>
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

mark::unit::modular::socket::socket(
	mark::unit::modular& parent,
	std::unique_ptr<module::base> module,
	mark::vector<int> pos):

	parent(parent), module(std::move(module)), pos(pos) {
	this->module->m_socket = this;
}

mark::unit::modular::socket::socket(mark::unit::modular::socket&& other):
	parent(other.parent), module(std::move(other.module)), pos(other.pos) {
	module->m_socket = this;
}

mark::unit::modular::socket& mark::unit::modular::socket::operator=(mark::unit::modular::socket&& other) {
	module = std::move(other.module);
	module->m_socket = this;
	pos = other.pos;
	return *this;
}

std::unique_ptr<mark::module::base> mark::unit::modular::socket::detach() {
	module->m_socket = nullptr;
	return std::move(module);
}

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
}

mark::unit::modular::modular(
	mark::world& world,
	mark::vector<double> pos,
	float rotation):
	mark::unit::base(world, pos), m_rotation(rotation) {}

void mark::unit::modular::tick(mark::tick_context& context) {
	if (this->dead()) {
		return;
	}
	{
		auto end_it = std::remove_if(
			m_sockets.begin(),
			m_sockets.end(),
			[&context](mark::unit::modular::socket& socket) {
			const auto dead = socket.module->dead();
			if (dead) {
				socket.module->on_death(context);
			}
			return dead;
		});
		m_sockets.erase(end_it, m_sockets.end());
	}
	for (auto& socket : m_sockets) {
		socket.module->tick(context);
	}

	double dt = context.dt;
	double speed = m_ai ? 64.0 : 320.0;
	if (mark::length(m_moveto - m_pos) > speed * dt) {
		const auto path = m_world.map().find_path(m_pos, m_moveto, 50.0);
#ifdef _DEBUG
		for (const auto& step : path) {
			mark::sprite::arguments args;
			args.image = m_world.resource_manager().image("generator.png");
			args.pos = step;
			context.sprites[100].emplace_back(args);
		}
#endif // !_DEBUG
		const auto dir = mark::normalize(m_moveto - m_pos);
		if (path.size() > 3) {
			const auto first = path[path.size() - 3];
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
	if (m_ai) {
		auto enemy = m_world.find_one(
			m_pos,
			1000.f,
			[this](const mark::unit::base& unit) {
			return unit.team() != this->team() && !unit.invincible();
		});
		if (enemy) {
			m_moveto = enemy->pos();
			m_lookat = enemy->pos();
			for (auto& socket : m_sockets) {
				socket.module->target(enemy->pos());
			}
		}
	}
	if (m_lookat != m_pos) {
		const auto direction = m_lookat - m_pos;
		const auto rot_diff = mark::atan(mark::rotate(direction, -m_rotation));
		const auto turn_speed = m_ai ? 32.f : 360.f;
		const auto rot_step = static_cast<float>(mark::sgn(rot_diff)  * turn_speed * dt);
		if (std::abs(rot_diff) < turn_speed * dt) {
			m_rotation = static_cast<float>(mark::atan(direction));
		} else {
			m_rotation += rot_step;
		}
	}

}


auto mark::unit::modular::get_attached(mark::module::base& module) ->
std::vector<std::reference_wrapper<mark::module::base>> {
	const auto pos = module.grid_pos();
	const auto size = mark::vector<int>(module.size());
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
	std::vector<std::reference_wrapper<mark::module::base>> out;
	for (auto& socket : m_sockets) {
		for (auto& pos : border) {
			if (::overlap(socket.pos, socket.module->size(), pos)) {
				out.push_back(*socket.module);
				break;
			}
		}
	}
	return out;
}


void mark::unit::modular::attach(
	std::unique_ptr<mark::module::base> module,
	mark::vector<int> pos) {

	if (!module) {
		throw mark::exception("NULL_MODULE");
	}
	// check collisions with other modules
	if (!can_attach(module, pos)) {
		throw mark::user_error("MODULE_OVERLAP");
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
	m_sockets.emplace_back(*this, std::move(module), pos);
}

auto mark::unit::modular::can_attach(
	const std::unique_ptr<module::base>& module,
	mark::vector<int> pos) const -> bool {

	if (!module) {
		return false;
	}
	for (auto& socket : m_sockets) {
		if (::overlap(socket.pos, socket.module->size(), pos)) {
			return false;
		}
	}
	return true;
}

auto mark::unit::modular::module(mark::vector<int> pos) const ->
	const mark::module::base*{

	for (auto& socket : m_sockets) {
		if (::overlap(socket.pos, socket.module->size(), pos)) {
			return socket.module.get();
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

	// check collisions with other modules
	auto socket_it = std::find_if(
		m_sockets.begin(),
		m_sockets.end(),
		[&pos](const mark::unit::modular::socket& socket) {
		return overlap(socket.pos, socket.module->size(), pos);
	});
	if (socket_it != m_sockets.end() && socket_it->module->detachable()) {
		auto out = socket_it->detach();
		if (socket_it != std::prev(m_sockets.end())) {
			*socket_it = std::move(m_sockets.back());
		}
		m_sockets.pop_back();
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
	} else if (command.type == mark::command::type::guide) {
		m_lookat = command.pos;
		for (auto& socket : m_sockets) {
			socket.module->target(command.pos);
		}
	} else if (command.type == mark::command::type::ai) {
		m_ai = true;
	} else if (command.type == mark::command::type::activate) {
		auto pad = m_world.find_one(
			m_pos,
			150.0,
			[this](const mark::unit::base& unit) {
			return !dynamic_cast<const mark::unit::landing_pad*>(&unit);
		});
		if (pad) {
			pad->activate(this->shared_from_this());
		}
	} else if (command.type == mark::command::type::shoot) {
		for (auto& socket : m_sockets) {
			socket.module->shoot(command.pos);
		}
	}
}

auto mark::unit::modular::dead() const -> bool {
	return m_core && m_core->dead();
}

void mark::unit::modular::on_death(mark::tick_context& context) {
	for (auto& socket : m_sockets) {
		context.units.emplace_back(std::make_shared<mark::unit::bucket>(
			m_world,
			m_pos,
			socket.detach()));
	}
}

bool mark::unit::modular::damage(const mark::idamageable::attributes& attr) {
	for (auto& socket : m_sockets) {
		if (socket.module->damage(attr)) {
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
	for (auto& socket : m_sockets) {
		auto& module = socket.module;
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
	return { damageable, min };
}

auto mark::unit::modular::collide(mark::vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<mark::idamageable>> {
	std::unordered_set<mark::idamageable*> out;
	// get shields
	std::vector<std::reference_wrapper<mark::module::shield_generator>> shields;
	for (auto& socket : m_sockets) {
		const auto shield_ptr = dynamic_cast<mark::module::shield_generator*>(socket.module.get());
		if (shield_ptr) {
			shields.push_back(*shield_ptr);
		}
	}
	for (auto& socket : m_sockets) {
		auto& module = socket.module;
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
		[](mark::idamageable* module) {
		return std::ref(*module);
	}
	);
	return tmp;
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
	for (auto& socket : m_sockets) {
		auto cargo = dynamic_cast<mark::module::cargo*>(socket.module.get());
		if (cargo) {
			out.push_back(*cargo);
		}
	}
	return out;
}
