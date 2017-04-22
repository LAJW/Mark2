#pragma once
#include "vector.h"

namespace mark {
	class command {
	public:
		enum class type {
			move,
			guide,
			ai,
			activate,
			shoot
		} type;
		mark::vector<double> pos;
	};
}