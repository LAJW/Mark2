#include "module_base.h"
#include <assert.h>

mark::module::base::base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail):
	m_size(size),
	m_thumbnail(thumbnail) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}

auto mark::module::base::collides(mark::vector<double> pos, float radius) const -> bool {
	const auto module_pos = this->socket()->relative_pos();
	const auto size = mark::vector<double>(m_size) * static_cast<double>(mark::module::size);
	const auto rotation = this->socket()->rotation();
	const auto orto = rotation + 90.f;
	const auto relative_to_module = pos - module_pos;
	return mark::distance(rotation, relative_to_module) < size.y / 2.0 + radius
		&& mark::distance(orto, relative_to_module) < size.x / 2.0 + radius;
}