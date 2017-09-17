#pragma once
#include "stdafx.h"

namespace mark {
struct tick_context;

namespace ui {

struct event;
class node {
public:
	virtual ~node() = default;
	virtual void tick(mark::tick_context& ctx) = 0;
	virtual bool click(const event&) = 0;
	virtual bool hover(const event&) = 0;
	// get absolute pos
	virtual auto pos() const noexcept -> vector<int> = 0;
};

} }