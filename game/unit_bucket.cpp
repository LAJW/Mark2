#include <algorithm>
#include <assert.h>
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
	const auto nearby_buckets = m_world.find(m_pos, size, [this](const mark::unit::base& unit) {
		return &unit != this && dynamic_cast<const mark::unit::bucket*>(&unit);
	});
	mark::vector<double> direction;
	if (!nearby_buckets.empty()) {
		for (const auto& bucket : nearby_buckets) {
			direction += bucket->pos();
		}
		if (direction == m_pos) {
			direction.x = context.random(-1.0, 1.0);
			direction.y = context.random(-1.0, 1.0);
		} else {
			direction = direction - m_pos;
			direction /= static_cast<double>(nearby_buckets.size());
			direction = mark::normalize(direction) * -1.0;
		}
		const auto new_pos = m_pos + direction * (mark::module::size * 2.0 * context.dt);
		if (m_world.map().traversable(m_pos, size)) {
			m_pos = new_pos;
		}
	}
	mark::sprite::info info;
	info.image = m_module->thumbnail();
	info.pos = m_pos;
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
	out << Key << "type" << Value << "unit_bucket";
	this->serialize_base(out);

	out << Key << "module" << Value << BeginMap;
	m_module->serialize(out);
	out << EndMap;

	out << EndMap;
}
