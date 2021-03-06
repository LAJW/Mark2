﻿#include "core.h"
#include <resource/image.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

mark::module::core::core(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_image(rm.image("core.png"))
{}

void mark::module::core::update(update_context& context)
{
	this->module::base::update(context);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos = this->pos();
		_.size = 32.f;
		_.rotation = parent_rotation();
		_.color = this->heat_color();
		return _;
	}());
}

auto mark::module::core::detachable() const -> bool { return false; }

auto mark::module::core::describe() const -> std::string
{
	return "Core Module\n" + base::describe();
}

void mark::module::core::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::module::core::passive() const noexcept -> bool { return true; }
