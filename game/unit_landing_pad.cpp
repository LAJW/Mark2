#include "stdafx.h"
#include "algorithm.h"
#include "unit_landing_pad.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "module_shield_generator.h"
#include "command.h"
#include "module_cargo.h"
#include "exception.h"

mark::unit::landing_pad::landing_pad(
	mark::world& world, mark::vector<double> pos)
	: mark::unit::base(world, pos)
	, m_image(world.resource_manager().image("landing-pad.png")) { }

// Serialize / Deserialize

mark::unit::landing_pad::landing_pad(
	mark::world& world,
	const YAML::Node& node)
	: mark::unit::base(world, node)
	, m_image(world.resource_manager().image("landing-pad.png"))
{
	if (const auto grabbed_node = node["grabbed"]) {
		m_grabbed = mark::module::deserialize(
			world.resource_manager(),
			grabbed_node);
	}
}

void mark::unit::landing_pad::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::landing_pad::type_name;
	this->serialize_base(out);
	if (const auto ship = m_ship.lock()) {
		out << Key << "ship_id" << Value << ship->id();
	}
	if (m_grabbed) {
		out << Key << "grabbed" << Value;
		m_grabbed->serialize(out);
	}
	out << EndMap;
}

// Methods

void mark::unit::landing_pad::tick(mark::tick_context& context)
{
	mark::sprite::info info;
	info.image = m_image;
	info.pos = pos();
	info.size = 320.f;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);
}

void mark::unit::landing_pad::dock(mark::unit::modular* ship)
{
	if (ship) {
		auto ship_ptr = std::dynamic_pointer_cast<mark::unit::modular>(
			m_world.find_one(pos(), 500.0,
			[ship](const mark::unit::base& unit) {
				return &unit == ship;
		}));
		m_ship = ship_ptr;
	} else {
		m_ship.reset();
	}
}

void mark::unit::landing_pad::activate(
	const std::shared_ptr<mark::unit::base>& by)
{
	if (const auto modular
		= std::dynamic_pointer_cast<mark::unit::modular>(by)) {
		m_ship = modular;
		m_world.target(this->shared_from_this());
		mark::command move;
		move.type = mark::command::type::move;
		move.pos = pos();
		modular->command(move);
		mark::command look;
		look.type = mark::command::type::guide;
		look.pos = pos() + mark::vector<double>(1, 0);
		modular->command(look);
	}
}

void mark::unit::landing_pad::command(const mark::command & command) {
	if (command.type == mark::command::type::activate && !command.release) {
		auto ship = m_ship.lock();
		if (ship) {
			ship->activate(this->shared_from_this());
			m_ship.reset();
		}
	} else if (command.type == mark::command::type::guide) {
		m_mousepos = command.pos;
	} else if (command.type == mark::command::type::move && !command.release) {
		auto ship = m_ship.lock();
		if (ship) {
			const auto relative = (command.pos - pos()) / 16.0;
			const auto module_pos = mark::round(relative);
			const auto pick_pos = mark::floor(relative);
			if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
				// ship drag&drop
				if (m_grabbed) {
					const auto drop_pos = module_pos - mark::vector<int>(m_grabbed->size()) / 2; // module's top-left corner
					try {
						ship->attach(m_grabbed, drop_pos);
					} catch (const mark::exception&) { /* no-op */ }
				} else {
					m_grabbed = ship->detach(pick_pos);
				}
			} else if (std::abs(relative.y) < 320.0 && relative.x < 320.0 + 16.0 * 16.0) {
				// cargo drag&drop
				double top = 0.0;
				for (auto& cargo_ref : ship->containers()) {
					auto& cargo = cargo_ref.get();
					const auto size = cargo.interior_size();
					const auto relative = command.pos - pos() + mark::vector<double>(-320 + 8, -top + 320 + 8);
					if (relative.y >= 0 && relative.y < size.y * 16) {
						if (m_grabbed) {
							const auto drop_pos = mark::round(relative / 16.0 - mark::vector<double>(m_grabbed->size()) / 2.0);
							if (cargo.drop(drop_pos, m_grabbed)
								== mark::error::code::success) {
								break;
							}
						} else {
							const auto pick_pos = mark::floor(relative / 16.0);
							m_grabbed = cargo.pick(pick_pos);
						}
						break;
					}
					top += size.y * 16.0 + 32.0;
				}
			}
		}
	} else if (!command.release) {
		if (command.type >= command::type::ability_0
			&& command.type <= command::type::ability_9
			|| command.type == command::type::shoot) {
			auto ship = m_ship.lock();
			if (ship) {
				const auto relative = (command.pos - pos()) / 16.0;
				const auto pick_pos = mark::floor(relative);
				ship->toggle_bind(command.type, pick_pos);
			}
		}
	}
}

auto mark::unit::landing_pad::collide(const mark::segment_t &) ->
	std::pair<mark::idamageable *, mark::vector<double>>
{ return { nullptr, { NAN, NAN } }; }

void mark::unit::landing_pad::resolve_ref(
	const YAML::Node& node,
	const std::unordered_map<uint64_t,
		std::weak_ptr<mark::unit::base>>& units)
{
	if (node["ship_id"]) {
		const auto ship_id = node["ship_id"].as<uint64_t>();
		m_ship = std::dynamic_pointer_cast<mark::unit::modular>(
			units.at(ship_id).lock());
	}
}

auto mark::unit::landing_pad::bindings() const ->
	mark::unit::modular::bindings_t
{
	const auto ship = m_ship.lock();
	assert(ship != nullptr);
	return ship->bindings();
}

auto mark::unit::landing_pad::ship() const ->
	std::shared_ptr<const mark::unit::modular>
{ return m_ship.lock(); }

auto mark::unit::landing_pad::grabbed() const noexcept ->
	const mark::module::base *
{ return m_grabbed.get(); }
