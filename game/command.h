#pragma once
#include "stdafx.h"

namespace mark {
namespace command {
struct move
{
	vector<double> to;
	bool release;
	vector<int> screen_pos;
};
struct guide
{
	vector<double> pos;
	vector<int> screen_pos;
};
struct use
{};
struct queue
{
	int8_t id;
	vector<double> pos;
};
struct activate
{
	int8_t id;
	vector<double> pos;
};
struct release
{
	int8_t id;
	vector<double> pos;
};
struct cancel
{};
using any = std::
	variant<move, guide, use, release, queue, activate, cancel, std::monostate>;
} // namespace command
}; // namespace mark
