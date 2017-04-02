#include "module_base.h"
#include <assert.h>

mark::module::base::base(mark::vector<unsigned> size)
	:m_size(size) {
	assert(size.x <= mark::module::max_dimension);
	assert(size.y <= mark::module::max_dimension);
}