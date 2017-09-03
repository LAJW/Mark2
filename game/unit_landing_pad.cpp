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
	, m_image(world.resource_manager().image("landing-pad.png")) { }

void mark::unit::landing_pad::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::landing_pad::type_name;
	this->serialize_base(out);
	if (const auto ship = m_ship.lock()) {
		out << Key << "ship_id" << Value << ship->id();
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

auto mark::unit::landing_pad::ship() -> std::shared_ptr<mark::unit::modular>
{ return m_ship.lock(); }

auto mark::unit::landing_pad::ship() const ->
	std::shared_ptr<const mark::unit::modular>
{ return m_ship.lock(); }