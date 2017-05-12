#pragma once
#include "vector.h"

namespace mark {
	class command {
	public:
		enum class type {
			null,
			move,
			guide,
			ai,
			activate,
			shoot,
			ability_1,
			ability_2,
			ability_3,
			ability_4,
			ability_5,
			ability_6,
			ability_7,
			ability_8,
			ability_9,
			ability_0,
			reset
		} type;
		mark::vector<double> pos;
		bool release;
	};
}