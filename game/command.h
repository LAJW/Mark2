#pragma once
#include "vector.h"

namespace mark {
	class command {
	public:
		enum class type {
			move,
			guide,
			ai
		} type;
		mark::vector<double> pos;
	};
}