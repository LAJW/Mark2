#pragma once
#include "stdafx.h"

namespace mark {
class command {
public:
	struct move { vector<double> to; };
	struct guide { vector<double> pos; };
	struct activate { };
	enum class type {
		null,
		shoot,
		ability_0,
		ability_1,
		ability_2,
		ability_3,
		ability_4,
		ability_5,
		ability_6,
		ability_7,
		ability_8,
		ability_9,
		reset
	} type;
	vector<double> pos;
	bool release = false;
	bool shift = false;
};

using command_any = std::variant<
	command::move, command::guide, command::activate, command, std::monostate>;
}