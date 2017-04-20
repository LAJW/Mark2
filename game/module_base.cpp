#include "module_base.h"
#include <assert.h>

mark::module::base::base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail):
	m_size(size),
	m_thumbnail(thumbnail) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}