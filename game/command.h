#pragma once
#include "stdafx.h"

namespace mark {
namespace command {
struct move
{
	vd to;
	bool release;
	bool shift;
	vi32 screen_pos;
};
struct guide
{
	vd pos;
	vi32 screen_pos;
};
struct use
{};
struct queue
{
	int8_t id;
	vd pos;
};
struct activate
{
	int8_t id;
	vd pos;
};
struct release
{
	int8_t id;
	vd pos;
};
struct cancel
{};
using any = std::
	variant<move, guide, use, release, queue, activate, cancel, std::monostate>;
} // namespace command
}; // namespace mark
