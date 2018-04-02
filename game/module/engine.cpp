﻿#include <stdafx.h>
#include "engine.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>

void mark::module::engine::tick(tick_context& context)
{
	this->module::base::tick(context);
	let pos = this->pos();
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = pos;
		_.size = module::size * 4.f;
		_.rotation = parent_rotation();
		_.color = this->heat_color();
		return _;
	}());
}

auto mark::module::engine::describe() const -> std::string { return "Engine"; }

auto mark::module::engine::global_modifiers() const -> module::modifiers
{
	module::modifiers mods;
	if (!m_stunned && m_state != state::off) {
		mods.velocity = 150.f;
	}
	return mods;
}

void mark::module::engine::command(const command::any& any)
{
	if (std::holds_alternative<command::queue>(any)) {
		m_state = m_state != state::off ? state::off : state::toggled;
	} else if (std::holds_alternative<command::activate>(any)) {
		m_state = state::manual;
	} else if (std::holds_alternative<command::release>(any)) {
		if (m_state != state::toggled) {
			m_state = state::off;
		}
	}
}

template <typename prop_man, typename T>
void mark::module::engine::bind(prop_man& property_manager, T& instance)
{
	(void)property_manager;
	(void)instance;
}

void mark::module::engine::bind(mark::property_manager& property_manager)
{
	bind(property_manager, *this);
	base::bind(property_manager);
}

mark::module::engine::engine(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_image_base(rm.image("engine.png"))
{}

void mark::module::engine::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::engine::reserved() const noexcept -> reserved_kind
{
	return reserved_kind::back;
}

auto mark::module::engine::passive() const noexcept -> bool { return false; }
