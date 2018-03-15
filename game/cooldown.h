#pragma once
#include "stdafx.h"

namespace mark {
class cooldown {
public:
	cooldown(float cooldown);
	void tick(double dt);
	bool ready();
	bool trigger(); // returns true if ready
private:
	float m_now = 0.f;
	const float m_max;
};
} // namespace mark