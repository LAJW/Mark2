#include "stdafx.h"
#include "unit_bucket.h"
#include "sprite.h"
#include "tick_context.h"
#include "module_base.h"
#include "world.h"
#include "map.h"

mark::unit::bucket::bucket(mark::world& world, const YAML::Node& node):
	mark::unit::base(world, node),
	m_module(mark::module::deserialize(world.resource_manager(), node["module"])) { }

mark::unit::bucket::bucket(mark::world& world, mark::vector<double> pos, std::unique_ptr<mark::module::base> module):
	mark::unit::base(world, pos),
	m_module(std::move(module)) {
	assert(m_module.get());
}

void mark::unit::bucket::tick(mark::tick_context& context) {
	if (this->dead()) { // dead bucket
		return;
	}
	const auto size = static_cast<float>(m_module->size().y, m_module->size().x) * mark::module::size;
	const auto nearby_buckets = m_world.find(
		pos(), size, [this](const mark::unit::base& unit) {
		return &unit != this && dynamic_cast<const mark::unit::bucket*>(&unit);
	});
	if (!nearby_buckets.empty()) {
		mark::vector<double> diff;
		for (const auto& bucket : nearby_buckets) {
			const auto vec = bucket->pos() - pos();
			const auto len = mark::length(vec);
			if (len != 0) {
				diff -= vec / (len * len);
			}
		}
		if (diff == mark::vector<double>(0, 0)) {
			m_direction = context.random<float>(-180.f, 180.f);
		} else {
			m_direction = static_cast<float>(mark::atan(diff));
		}
		const auto ds = mark::rotate(mark::vector<double>(30.0 * context.dt, 0), m_direction);
		if (m_world.map().traversable(pos() + ds, size)) {
			this->pos(pos() + ds);
		}
	}
	mark::sprite::info info;
	info.image = m_module->thumbnail();
	info.pos = pos();
	info.size = size;
	context.sprites[1].emplace_back(info);
}

auto mark::unit::bucket::dead() const -> bool {
	return m_module == nullptr;
}

auto mark::unit::bucket::damage(const mark::idamageable::info& attr) -> bool {
	return false;
}

auto mark::unit::bucket::invincible() const -> bool {
	return true;
}

auto mark::unit::bucket::collide(const mark::segment_t &) ->
	std::pair<mark::idamageable *, mark::vector<double>> {
	return { nullptr, { NAN, NAN } };
}

auto mark::unit::bucket::release() -> std::unique_ptr<mark::module::base> {
	return std::move(m_module);
}

void mark::unit::bucket::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::bucket::type_name;
	this->serialize_base(out);

	out << Key << "module" << Value << BeginMap;
	m_module->serialize(out);
	out << EndMap;

	out << EndMap;
}
