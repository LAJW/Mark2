#include "bucket.h"
#include <exception.h>
#include <map.h>
#include <module/base.h>
#include <random.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>
#include <world.h>

mark::unit::bucket::bucket(
	mark::world& world,
	mark::random& random,
	const YAML::Node& node)
	: unit::base(world, node)
	, m_item(
		  module::deserialize(world.resource_manager(), random, node["module"]))
	, m_rotation(node["rotation"].as<float>(m_rotation))
{
}

mark::unit::bucket::bucket(info info)
	: unit::base(info)
	, m_item(move(info.item))
	, m_rotation(info.rotation)
{
	Expects(m_item);
	let module = dynamic_cast<const module::base*>(m_item.get());
	Expects(!module || !module->dead());
}

void mark::unit::bucket::update(update_context& context)
{
	if (this->dead()) {
		return;
	}
	let size = this->radius() * 2.;
	let nearby_buckets = world().find<unit::bucket>(
		pos(), size, [this](const unit::base& unit) { return &unit != this; });
	if (!nearby_buckets.empty()) {
		vd diff;
		for (let& bucket : nearby_buckets) {
			let vec = bucket->pos() - pos();
			let len = length(vec);
			if (len != 0) {
				diff -= vec / (len * len);
			}
		}
		if (diff == vd(0, 0)) {
			m_direction = context.random(-180.f, 180.f);
		} else {
			m_direction = gsl::narrow_cast<float>(atan(diff));
		}
		let ds = rotate(vd(30.0 * context.dt, 0), m_direction);
		if (world().map().traversable(pos() + ds, size)) {
			this->pos(pos() + ds);
		}
	}
	context.sprites[1].emplace_back([&] {
		sprite _;
		_.image = m_item->thumbnail();
		_.pos = pos();
		_.rotation = m_rotation;
		_.size = gsl::narrow_cast<float>(size);
		return _;
	}());
}

auto mark::unit::bucket::dead() const -> bool { return m_item == nullptr; }

auto mark::unit::bucket::release() -> interface::item_ptr
{
	return std::move(m_item);
}

void mark::unit::bucket::insert(interface::item_ptr item)
{
	if (m_item) {
		throw exception("BUCKET_FULL");
	}
	m_item = std::move(item);
}

void mark::unit::bucket::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::bucket::type_name;
	base::serialize(out);

	// TODO: Change to item
	out << Key << "module" << Value;
	m_item->serialize(out);

	out << Key << "rotation" << Value << m_rotation;

	out << EndMap;
}

auto mark::unit::bucket::radius() const -> double
{
	if (!m_item) {
		return 0.;
	}
	return static_cast<float>(std::max(m_item->size().y, m_item->size().x))
		* module::size / 2.;
}
