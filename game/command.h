#pragma once
#include "stdafx.h"

namespace mark {
namespace command {
struct move { vector<double> to; bool release; };
struct guide { vector<double> pos; };
struct use { };
enum class type {
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
	ability_9
};
struct queue { type type; vector<double> pos; };
struct activate { type type; vector<double> pos; };
struct release { type type; vector<double> pos; };
using any = std::variant<
	move, guide, use, release, queue, activate, std::monostate>;
}
};
