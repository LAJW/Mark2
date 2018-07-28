#include "shard.h"
#include <exception.h>
#include <module/base.h>
#include <resource_manager.h>
#include <stdafx.h>

mark::item::shard::shard(mark::resource::manager& rm, const YAML::Node& node)
	: m_thumbnail(rm.image("ruby.png"))
	, m_quantity(node["quantity"].as<size_t>(1))
{}

auto mark::item::shard::describe() const -> std::string
{
	std::ostringstream os;
	os << "Shard" << std::endl;
	os << "Used to construct new items and gems" << std::endl;
	os << "Quantity: " << m_quantity << std::endl;
	return os.str();
}

void mark::item::shard::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	out << Key << "quantity" << Value << m_quantity;
	out << EndMap;
}

auto mark::item::shard::size() const -> vu32 { return { 1, 1 }; }

auto mark::item::shard::thumbnail() const -> resource::image_ptr
{
	return m_thumbnail;
}

void mark::item::shard::stack(interface::item_ptr& item)
{
	let constexpr max_stack_quantity = 20;
	if (let other = dynamic_cast<shard*>(item.get())) {
		let total = m_quantity + other->quantity();
		if (total > max_stack_quantity) {
			other->m_quantity = total - max_stack_quantity;
			m_quantity = max_stack_quantity;
		} else {
			item.reset();
			m_quantity = total;
		}
	}
}

auto mark::item::shard::can_stack(const interface::item& item) const -> bool
{
	return dynamic_cast<const shard*>(&item) != nullptr && m_quantity < 20;
}

auto mark::item::shard::quantity() const -> size_t
{
	return m_quantity;
}
