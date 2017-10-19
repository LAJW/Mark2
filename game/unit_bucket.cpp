#include "stdafx.h"
#include "exception.h"
#include "unit_bucket.h"
#include "sprite.h"
#include "tick_context.h"
#include "module_base.h"
#include "world.h"
#include "map.h"

mark::unit::bucket::bucket(world& world, const YAML::Node& node):
	unit::base(world, node),
	m_module(module::deserialize(world.resource_manager(), node["module"])) { }

mark::unit::bucket::bucket(world& world, vector<double> pos, std::unique_ptr<module::base> module) :
	unit::base(world, pos),
	m_module(std::move(module)) {
	assert(m_module != nullptr);
	if (m_module->dead()) {
		throw std::exception("DEAD_MODULE_IN_BUCKET");
	}
}

void mark::unit::bucket::tick(tick_context& context) {
	if (this->dead()) { // dead bucket
		return;
	}
	const auto size = static_cast<float>(m_module->size().y, m_module->size().x) * module::size;
	const auto nearby_buckets = m_world.find(
		pos(), size, [this](const unit::base& unit) {
		return &unit != this && dynamic_cast<const unit::bucket*>(&unit);
	});
	if (!nearby_buckets.empty()) {
		vector<double> diff;
		for (const auto& bucket : nearby_buckets) {
			const auto vec = bucket->pos() - pos();
			const auto len = length(vec);
			if (len != 0) {
				diff -= vec / (len * len);
			}
		}
		if (diff == vector<double>(0, 0)) {
			m_direction = context.random<float>(-180.f, 180.f);
		} else {
			m_direction = static_cast<float>(atan(diff));
		}
		const auto ds = rotate(vector<double>(30.0 * context.dt, 0), m_direction);
		if (m_world.map().traversable(pos() + ds, size)) {
			this->pos(pos() + ds);
		}
	}
	sprite info;
	info.image = m_module->thumbnail();
	info.pos = pos();
	info.size = size;
	context.sprites[1].emplace_back(info);
}

auto mark::unit::bucket::dead() const -> bool {
	return m_module == nullptr;
}

auto mark::unit::bucket::release() -> std::unique_ptr<module::base> {
	return std::move(m_module);
}

void mark::unit::bucket::insert(std::unique_ptr<module::base> module) {
	if (m_module) {
		throw exception("BUCKET_FULL");
	}
	m_module = std::move(module);
}

void mark::unit::bucket::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::bucket::type_name;
	this->serialize_base(out);

	out << Key << "module" << Value;
	m_module->serialize(out);

	out << EndMap;
}
