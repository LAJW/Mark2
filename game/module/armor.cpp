#include "armor.h"
#include <resource_image.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

mark::module::armor::armor(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_image(rm.image("armor.png"))
{}

void mark::module::armor::update(update_context& context)
{
	this->module::base::update(context);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos = this->pos();
		_.size = 16.f;
		_.rotation = parent_rotation();
		_.color = this->heat_color();
		return _;
	}());
}

auto mark::module::armor::describe() const -> std::string
{
	return "Armor Module\n" + base::describe();
}

void mark::module::armor::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::module::armor::passive() const noexcept -> bool { return true; }

auto mark::module::armor::local_modifiers() const -> module::modifiers
{
	auto mods = base::local_modifiers();
	mods.armor = m_armor;
	return mods;
}
