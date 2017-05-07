#include <array>
#include <assert.h>
#include "module_base.h"
#include "exception.h"
#include "tick_context.h"
#include "world.h"

mark::module::base::base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail):
	m_size(size),
	m_thumbnail(thumbnail) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}

auto mark::module::base::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable*, mark::vector<double>> {
	const auto size = this->size();
	// half width
	const auto hw = static_cast<double>(size.x) / 2.0 * mark::module::size;
	// half height
	const auto hh = static_cast<double>(size.y) / 2.0 * mark::module::size;
	const auto pos = this->pos();
	const auto rotation = parent().rotation();
	const std::array<segment_t, 4> segments {
		mark::segment_t{ { -hw, -hh }, { -hw, hh } },  // left
		mark::segment_t{ { -hw, hh },  { hw, hh } },   // bottom
		mark::segment_t{ { hw, hh },   { hw, -hh } },  // right
		mark::segment_t{ { hw, -hh },  { -hw, -hh } }  // side
	};
	auto min = mark::vector<double>(NAN, NAN);
	double min_length = 40000.0;
	for (const auto& raw : segments) {
		const auto segment = std::make_pair(
			mark::rotate(raw.first, rotation) + pos,
			mark::rotate(raw.second, rotation) + pos
		);
		const auto intersection = intersect(segment, ray);
		if (!std::isnan(intersection.x)) {
			const auto length = mark::length(intersection - pos);
			if (length < min_length) {
				min_length = length;
				min = intersection;
			}
		}
	}
	if (!std::isnan(min.x)) {
		return { this, min };
	} else {
		return { nullptr, min };
	}
}

auto mark::module::base::neighbours() -> std::vector<std::reference_wrapper<mark::module::base>> {
	return parent().get_attached(*this);
}

auto mark::module::base::grid_pos() const noexcept -> mark::vector<int> {
	return mark::vector<int>(m_grid_pos);
}

bool mark::module::base::damage(const mark::idamageable::attributes & attr) {
	if (attr.team != parent().team() && m_cur_health > 0
		&& attr.damaged->find(this) == attr.damaged->end()) {
		attr.damaged->insert(this);
		m_cur_health -= attr.physical;
		return true;
	}
	return false;
}

auto mark::module::base::dead() const -> bool {
	return m_cur_health <= 0.f;
}

void mark::module::base::on_death(mark::tick_context& context) {
	mark::tick_context::spray_info spray;
	spray.image = parent().world().resource_manager().image("explosion.png");
	spray.pos = pos();
	spray.velocity(75.f, 150.f);
	spray.lifespan(0.3f);
	spray.diameter(24.f);
	spray.count = 20;
	context.render(spray);
}

auto mark::module::base::parent() const -> const mark::unit::modular& {
	if (m_parent) {
		return *m_parent;
	} else {
		throw mark::exception("NO_PARENT");
	}
}

auto mark::module::base::parent() -> mark::unit::modular&{
	return const_cast<mark::unit::modular&>(static_cast<const mark::module::base*>(this)->parent());
}

auto mark::module::base::pos() const -> mark::vector<double> {
	const auto pos = (mark::vector<float>(grid_pos()) + mark::vector<float>(this->size()) / 2.f)
		* static_cast<float>(mark::module::size);
	return parent().pos() + mark::vector<double>(rotate(pos, parent().rotation()));
}

auto mark::module::base::thumbnail() const -> std::shared_ptr<const mark::resource::image> {
	return m_thumbnail;
}

auto mark::module::base::size() const -> mark::vector<unsigned> {
	return m_size;
}
