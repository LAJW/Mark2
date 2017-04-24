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

mark::unit::modular::socket::socket(mark::unit::modular& parent, std::unique_ptr<module::base> module, mark::vector<int> pos)
	:m_parent(parent), m_module(std::move(module)), m_pos(pos) {
	m_module->socket(this);
}

mark::unit::modular::socket::socket(mark::unit::modular::socket&& other)
	: m_parent(other.m_parent), m_module(std::move(other.m_module)), m_pos(other.m_pos) {
	m_module->socket(this);
}

mark::unit::modular::socket& mark::unit::modular::socket::operator=(mark::unit::modular::socket&& other) {
	m_module = std::move(other.m_module);
	m_module->socket(this);
	m_pos = other.m_pos;
	return *this;
}

mark::unit::modular::socket::~socket() {

}

auto mark::unit::modular::socket::get_attached() ->
std::vector<std::reference_wrapper<mark::module::base>> {
	std::vector<std::reference_wrapper<mark::module::base>> out;
	for (auto socket : m_parent.get_attached(*this, m_pos)) {
		out.push_back(*socket.get().m_module);
	}
	return out;
}

void mark::unit::modular::socket::tick(mark::tick_context& context) {
	m_module->tick(context);
}

inline auto mark::unit::modular::socket::size() const->mark::vector<unsigned> {
	return m_module->size();
}

auto mark::unit::modular::socket::relative_pos() const->mark::vector<double> {
	const auto pos = (mark::vector<float>(m_pos) + mark::vector<float>(this->size()) / 2.f)
		* static_cast<float>(mark::module::size);
	return m_parent.pos() + mark::vector<double>(rotate(pos, m_parent.rotation()));
}

auto mark::unit::modular::socket::rotation() const -> float {
	return m_parent.rotation();
}

auto mark::unit::modular::socket::dead() const -> bool {
	return m_module->dead();
}

auto mark::unit::modular::socket::module() -> mark::module::base& {
	return *m_module;
}

auto mark::unit::modular::socket::module() const->const mark::module::base & {
	return *m_module;
}

std::unique_ptr<mark::module::base> mark::unit::modular::socket::detach() {
	m_module->socket(nullptr);
	return std::move(m_module);
}

// MODULAR

mark::unit::modular::modular(mark::world& world, mark::vector<double> pos, float rotation)
	:mark::unit::base(world, pos), m_rotation(rotation) {

}

void mark::unit::modular::tick(mark::tick_context& context) {
	for (auto& socket : m_sockets) {
		socket.tick(context);
	}
	double dt = context.dt;
	double speed = m_ai ? 64.0 : 320.0;
	if (mark::length(m_moveto - m_pos) > speed * dt) {
		const auto path = m_world.map().find_path(m_pos, m_moveto, 50.0);
#ifdef _DEBUG
		for (const auto& step : path) {
			context.sprites[100].emplace_back(m_world.resource_manager().image("generator.png"), step);
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
		auto enemy = m_world.find_one(m_pos, 1000.f, [this](const mark::unit::base& unit) {
			return unit.team() != this->team() && !unit.invincible();
		});
		if (enemy) {
			m_moveto = enemy->pos();
			m_lookat = enemy->pos();
			for (auto& socket : m_sockets) {
				socket.module().target(enemy->pos());
			}
		}
	}
	if (m_lookat != m_pos) {
		const auto direction = mark::normalize((m_lookat - m_pos));
		const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -m_rotation)));
		const auto rot_step = static_cast<float>(turn_direction  * 32.f * dt);
		if (std::abs(mark::atan(mark::rotate(direction, -m_rotation))) < 32.f * dt) {
			m_rotation = static_cast<float>(mark::atan(direction));
		} else {
			m_rotation += rot_step;
		}
	}

}


auto mark::unit::modular::get_attached(const socket&, mark::vector<int>) ->
std::vector<std::reference_wrapper<mark::unit::modular::socket>> {
	std::vector<std::reference_wrapper<mark::unit::modular::socket>> out;
	for (auto& socket : m_sockets) {
		// TODO
	}
	return out;
}


void mark::unit::modular::attach(std::unique_ptr<mark::module::base> module, mark::vector<int> pos) {
	if (!module) {
		throw mark::exception("NULL_MODULE");
	}
	// check collisions with other modules
	auto insert_right = pos.x + static_cast<int>(module->size().x);
	auto insert_bottom = pos.y + static_cast<int>(module->size().y);
	for (auto& socket : m_sockets) {
		auto socket_right = socket.pos().x + static_cast<int>(socket.size().x);
		auto socket_bottom = socket.pos().y + static_cast<int>(socket.size().y);
		if (pos.x < socket_right && insert_right > socket.pos().x
			&& pos.y < socket_bottom && insert_bottom > socket.pos().y) {
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
	m_sockets.emplace_back(*this, std::move(module), pos);
}

auto mark::unit::modular::can_attach(const std::unique_ptr<module::base>& module, mark::vector<int> pos) const -> bool {
	if (!module) {
		return false;
	}
	auto insert_right = pos.x + static_cast<int>(module->size().x);
	auto insert_bottom = pos.y + static_cast<int>(module->size().y);
	for (auto& socket : m_sockets) {
		auto socket_right = socket.pos().x + static_cast<int>(socket.size().x);
		auto socket_bottom = socket.pos().y + static_cast<int>(socket.size().y);
		if (pos.x < socket_right && insert_right > socket.pos().x
			&& pos.y < socket_bottom && insert_bottom > socket.pos().y) {
			return false;
		}
	}
	return true;
}

auto mark::unit::modular::detach(mark::vector<int> pos)->std::unique_ptr<mark::module::base> {
	// check collisions with other modules
	auto socket_it = std::find_if(m_sockets.begin(), m_sockets.end(), [&pos](const mark::unit::modular::socket& socket) {
		auto socket_right = socket.pos().x + static_cast<int>(socket.size().x);
		auto socket_bottom = socket.pos().y + static_cast<int>(socket.size().y);
		return pos.x < socket_right && pos.x >= socket.pos().x
			&& pos.y < socket_bottom && pos.y >= socket.pos().y;
	});
	if (socket_it != m_sockets.end()) {
		if (socket_it->module().detachable()) {
			auto out = socket_it->detach();
			if (socket_it != std::prev(m_sockets.end())) {
				*socket_it = std::move(m_sockets.back());
			}
			m_sockets.pop_back();
			return std::move(out);
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

namespace mark {

}

auto mark::unit::modular::collides(mark::vector<double> pos, float radius) const -> bool {
	for (const auto& socket : m_sockets) {
		const auto size = mark::vector<double>(socket.size()) * 16.0;
		const auto module_pos = socket.relative_pos();
		const auto orto = m_rotation + 90.f;
		const auto relative_to_module = pos - module_pos;
		if (mark::distance(m_rotation, relative_to_module) < size.y / 2.0 + radius
			&& mark::distance(orto, relative_to_module) < size.x / 2.0 + radius) {
			return true;
		}
	}
	return false;
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
			socket.module().target(command.pos);
		}
	} else if (command.type == mark::command::type::ai) {
		m_ai = true;
	} else if (command.type == mark::command::type::activate) {
		auto pad = m_world.find_one(m_pos, 150.0, [this](const mark::unit::base& unit) {
			return dynamic_cast<const mark::unit::landing_pad*>(&unit) != nullptr;
		});
		if (pad) {
			pad->activate(this->shared_from_this());
		}
	} else if (command.type == mark::command::type::shoot) {
		for (auto& socket : m_sockets) {
			socket.module().shoot(command.pos);
		}
	}
}

auto mark::unit::modular::dead() const -> bool {
	return m_core && m_core->dead();
}

auto mark::unit::modular::invincible() const -> bool {
	return false;
}

void mark::unit::modular::activate(const std::shared_ptr<mark::unit::base>& by) {
	m_world.target(this->shared_from_this());
}

auto mark::unit::modular::containers() -> std::vector<std::reference_wrapper<mark::module::cargo>> {
	std::vector<std::reference_wrapper<mark::module::cargo>> out;
	for (auto& socket : m_sockets) {
		auto cargo = dynamic_cast<mark::module::cargo*>(&socket.module());
		if (cargo) {
			out.push_back(*cargo);
		}
	}
	return out;
}
