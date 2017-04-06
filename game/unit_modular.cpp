#include <assert.h>
#include <cmath>
#include "unit_modular.h"
#include "module_base.h"
#include "module_core.h"
#include "exception.h"
#include "sprite.h"
#include "world.h"
#include "terrain_base.h"
#include "command.h"
#include "find_path.h"



mark::unit::modular::socket::socket(mark::unit::modular& parent, std::unique_ptr<module::base> module, mark::vector<int> pos)
	:m_parent(parent), m_module(std::move(module)), m_pos(pos) {
	m_module->socket(this);
}

mark::unit::modular::socket::socket(mark::unit::modular::socket&& other)
	:m_parent(other.m_parent), m_module(std::move(other.m_module)), m_pos(other.m_pos) {
	m_module->socket(this);
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

auto mark::unit::modular::socket::render() const -> std::vector<mark::sprite> {
	return m_module->render();
}

inline auto mark::unit::modular::socket::size() const->mark::vector<unsigned> {
	return m_module->size();
}

auto mark::unit::modular::socket::relative_pos() const->mark::vector<double> {
	return m_parent.pos() + mark::vector<double>(rotate(mark::vector<float>(m_pos) * static_cast<float>(mark::module::size), m_parent.rotation()));
}

auto mark::unit::modular::socket::rotation() const -> float {
	return m_parent.rotation();
}

// MODULAR

mark::unit::modular::modular(mark::world& world, mark::vector<double> pos, float rotation)
	:mark::unit::base(world, pos), m_rotation(rotation) {

}

void mark::unit::modular::tick(double dt) {
	if (m_command.type == mark::command::type::move) {
		if (mark::length(m_command.pos - pos()) > 320.0 * dt) {
			const auto map_pos = mark::vector<int>(std::ceil(pos().x / 32.0), std::ceil(pos().y / 32.0)) + mark::vector<int>(500, 500);
			const auto path = mark::find_path(m_world.map(), map_pos, mark::vector<int>(m_command.pos) / 32 + mark::vector<int>(500, 500));
			if (path.size() > 3) {
				const auto first = mark::vector<double>(path[path.size() - 3] - mark::vector<int>(500, 500)) * 32.0 + mark::vector<double>(16.0, 16.0);
				m_pos += mark::normalize((first - pos())) * 320.0 * dt;
			} else if (mark::length(m_command.pos - m_pos) > 320.0 * dt) {
				const auto step = mark::normalize(m_command.pos - m_pos) * 320.0 * dt;
				const auto map_pos = mark::vector<int>(std::ceil((step.x + pos().x)/ 32.0), std::ceil(step.y + pos().y / 32.0)) + mark::vector<int>(500, 500);
				if (map_pos.x > 0 && map_pos.x < 1000 && map_pos.y > 0
					&& map_pos.y < 1000 && m_world.map()[map_pos.x][map_pos.y]
					&& m_world.map()[map_pos.x][map_pos.y]->traversable()) {
					m_pos += step;
				}
			}
		} else {
			m_pos = m_command.pos;
		}
	}

	m_rotation += 30.0 * dt;
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
	assert(module);
	// check collisions with other modules
	auto insert_right = pos.x + static_cast<int>(module->size().x);
	auto insert_bottom = pos.x + static_cast<int>(module->size().y);
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

auto mark::unit::modular::get_core() -> mark::module::core& {
	if (m_core) {
		return *m_core;
	} else {
		throw mark::user_error("NO_CORE");
	}
}

auto mark::unit::modular::render() const -> std::vector<mark::sprite> {
	std::vector<mark::sprite> sprites;
	for (auto& socket : m_sockets) {
		auto socket_sprites = socket.render();
		sprites.insert(
			sprites.end(),
			std::make_move_iterator(socket_sprites.begin()),
			std::make_move_iterator(socket_sprites.end())
		);
	}
	return sprites;
}

void mark::unit::modular::command(const mark::command& command) {
	m_command = command;
}