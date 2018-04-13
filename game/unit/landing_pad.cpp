#include <stdafx.h>
#include "landing_pad.h"
#include "modular.h"
#include <algorithm.h>
#include <command.h>
#include <exception.h>
#include <module/cargo.h>
#include <module/shield_generator.h>
#include <resource_manager.h>
#include <sprite.h>
#include <update_context.h>
#include <world.h>

constexpr let LANDING_PAD_SIZE = 320.f;

mark::unit::landing_pad::landing_pad(const info& info)
	: activable(info)
	, m_image(info.world->resource_manager().image("landing-pad.png"))
{}

// Serialize / Deserialize

mark::unit::landing_pad::landing_pad(mark::world& world, const YAML::Node& node)
	: activable(world, node)
	, m_image(world.resource_manager().image("landing-pad.png"))
{}

void mark::unit::landing_pad::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::landing_pad::type_name;
	base::serialize(out);
	if (let ship = m_ship.lock()) {
		out << Key << "ship_id" << Value << reinterpret_cast<size_t>(this);
	}
	out << EndMap;
}

// Methods

void mark::unit::landing_pad::update(mark::update_context& context)
{
	sprite info;
	info.image = m_image;
	info.pos = pos();
	info.size = LANDING_PAD_SIZE;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);
}

auto mark::unit::landing_pad::use(const std::shared_ptr<unit::modular>& ship)
	-> std::error_code
{
	m_ship = ship;
	world().target(this->shared_from_this());
	ship->command(command::move{ pos() });
	ship->command(command::guide{ pos() + vector<double>(1, 0) });
	return error::code::success;
}

void mark::unit::landing_pad::command(const mark::command::any& any)
{
	if (std::holds_alternative<command::use>(any)) {
		if (auto ship = m_ship.lock()) {
			world().target(std::move(ship));
			m_ship.reset();
		}
	}
}

void mark::unit::landing_pad::resolve_ref(
	const YAML::Node& node,
	const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units)
{
	if (node["ship_id"]) {
		let ship_id = node["ship_id"].as<uint64_t>();
		m_ship =
			std::dynamic_pointer_cast<unit::modular>(units.at(ship_id).lock());
	}
}

auto mark::unit::landing_pad::bindings() const -> unit::modular::bindings_t
{
	let ship = m_ship.lock();
	assert(ship != nullptr);
	return ship->bindings();
}

auto mark::unit::landing_pad::ship() -> std::shared_ptr<unit::modular>
{
	return m_ship.lock();
}

auto mark::unit::landing_pad::ship() const
	-> std::shared_ptr<const unit::modular>
{
	return m_ship.lock();
}