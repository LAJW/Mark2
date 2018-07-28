#include "shard.h"
#include <exception.h>
#include <module/base.h>
#include <resource_manager.h>
#include <stdafx.h>

mark::item::shard::shard(mark::resource::manager& rm, const YAML::Node& node)
	: base(node)
	, m_thumbnail(rm.image("ruby.png"))
{}

auto mark::item::shard::describe() const -> std::string
{
	std::ostringstream os;
	os << "Shard" << std::endl;
	os << "Used to construct new items and gems" << std::endl;
	os << "Quantity: " << this->quantity() << std::endl;
	return os.str();
}

void mark::item::shard::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::item::shard::size() const -> vu32 { return { 1, 1 }; }

auto mark::item::shard::thumbnail() const -> resource::image_ptr
{
	return m_thumbnail;
}
auto mark::item::shard::type_equals(const base& item) const -> bool
{
	return dynamic_cast<const shard*>(&item) != nullptr;
}

auto mark::item::shard::max_stack_size() const -> size_t { return 20; }
