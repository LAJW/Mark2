#include <stdafx.h>
#include "core.h"
#include <resource_image.h>
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>

mark::module::core::core(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_image(rm.image("core.png"))
{
}

void mark::module::core::tick(tick_context& context)
{
	this->module::base::tick(context);
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
	return "Core Module";
}

void mark::module::core::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::core::passive() const noexcept -> bool { return true; }
