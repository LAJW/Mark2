#include "module_base.h"
#include "exception.h"
#include <assert.h>

mark::module::base::base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail):
	m_size(size),
	m_thumbnail(thumbnail) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}

auto mark::module::base::collides(mark::vector<double> pos, float radius) const -> bool {
	const auto module_pos = this->pos();
	const auto size = mark::vector<double>(m_size) * static_cast<double>(mark::module::size);
	const auto rotation = parent().rotation();
	const auto orto = rotation + 90.f;
	const auto relative_to_module = pos - module_pos;
	return mark::distance(rotation, relative_to_module) < size.y / 2.0 + radius
		&& mark::distance(orto, relative_to_module) < size.x / 2.0 + radius;
}

auto mark::module::base::neighbours() -> std::vector<std::reference_wrapper<mark::module::base>> {
	return parent().get_attached(*this);
}

auto mark::module::base::grid_pos() -> const mark::vector<int>
{
	return m_socket->m_pos;
}

auto mark::module::base::parent() const -> const mark::unit::modular& {
	if (m_socket) {
		return m_socket->m_parent;
	} else {
		throw mark::exception("NO_PARENT");
	}
}

auto mark::module::base::parent() -> mark::unit::modular&{
	return const_cast<mark::unit::modular&>(static_cast<const mark::module::base*>(this)->parent());
}

auto mark::module::base::pos() const -> mark::vector<double> {
	const auto pos = (mark::vector<float>(m_socket->m_pos) + mark::vector<float>(this->size()) / 2.f)
		* static_cast<float>(mark::module::size);
	return parent().pos() + mark::vector<double>(rotate(pos, parent().rotation()));
}

auto mark::module::base::thumbnail() const -> std::shared_ptr<const mark::resource::image> {
	return m_thumbnail;
}

auto mark::module::base::size() const -> mark::vector<unsigned> {
	return m_size;
}
