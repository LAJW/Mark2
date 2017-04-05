#pragma once
#include "vector.h"

namespace mark {
	class command {
	public:
		enum class type {
			move
		} type;
		mark::vector<double> pos;
	};
}