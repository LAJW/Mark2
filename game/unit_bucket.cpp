#include <algorithm>
#include <assert.h>
#include "unit_bucket.h"
#include "sprite.h"
#include "tick_context.h"
#include "module_base.h"

mark::unit::bucket::bucket(mark::world& world, mark::vector<double> pos, std::unique_ptr<mark::module::base> module):
	mark::unit::base(world, pos),
	m_module(std::move(module)) {
	assert(module.get());
}

void mark::unit::bucket::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto size = static_cast<float>(m_module->size().y, m_module->size().x) * mark::module::size;
	context.sprites[1].push_back(mark::sprite(m_module->thumbnail(), m_pos, size));
}

auto mark::unit::bucket::dead() const -> bool {
	return false;
}

auto mark::unit::bucket::damage(const mark::idamageable::attributes& attr) -> bool {
	return false;
}

auto mark::unit::bucket::invincible() const -> bool {
	return true;
}

auto mark::unit::bucket::collides(mark::vector<double> pos, float radius) const -> bool {
	return false;
}
