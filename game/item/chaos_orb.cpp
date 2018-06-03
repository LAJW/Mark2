#include "chaos_orb.h"
#include <exception.h>
#include <module/base.h>
#include <resource_manager.h>
#include <stdafx.h>

mark::item::chaos_orb::chaos_orb(
	mark::resource::manager& rm,
	const YAML::Node& node)
	: m_thumbnail(rm.image("chaos-orb.png"))
	, m_quantity(node["quantity"].as<size_t>(1))
{}

auto mark::item::chaos_orb::use_on(
	resource::manager& rm,
	const std::unordered_map<std::string, YAML::Node>& blueprints,
	module::base& item) -> use_on_result
{
	Expects(m_quantity != 0);
	let error = item.randomise(blueprints, rm);
	if (error == error::code::success) {
		--m_quantity;
	}
	use_on_result result;
	result.error = error;
	result.consumed = m_quantity == 0;
	return result;
}

auto mark::item::chaos_orb::describe() const -> std::string
{
	std::ostringstream os;
	os << "Chaos Orb" << std::endl;
	os << "Randomises all random properties of an item." << std::endl;
	os << "Quantity: " << m_quantity << std::endl;
	return os.str();
}

void mark::item::chaos_orb::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	out << Key << "quantity" << Value << m_quantity;
	out << EndMap;
}

auto mark::item::chaos_orb::size() const -> vu32 { return { 2, 2 }; }

auto mark::item::chaos_orb::thumbnail() const -> resource::image_ptr
{
	return m_thumbnail;
}

void mark::item::chaos_orb::stack(interface::item_ptr& item)
{
	if (let other = dynamic_cast<chaos_orb*>(item.get())) {
		m_quantity += other->m_quantity;
		if (m_quantity > 20) {
			other->m_quantity = m_quantity - 20;
		} else {
			item.reset();
		}
	}
}

auto mark::item::chaos_orb::can_stack(const interface::item& item) const -> bool
{
	return dynamic_cast<const chaos_orb*>(&item) != nullptr && m_quantity < 20;
}

auto mark::item::chaos_orb::quantity() const -> size_t
{
	return m_quantity;
}