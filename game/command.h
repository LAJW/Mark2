#pragma once
#include "stdafx.h"

namespace mark {
namespace command {
struct move { vector<double> to; bool release; };
struct guide { vector<double> pos; };
struct use { };
struct queue { int8_t id; vector<double> pos; };
struct activate { int8_t id; vector<double> pos; };
struct release { int8_t id; vector<double> pos; };
using any = std::variant<
	move, guide, use, release, queue, activate, std::monostate>;
}
};
