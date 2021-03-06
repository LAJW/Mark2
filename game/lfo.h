﻿#pragma once
#include "stdafx.h"

namespace mark {
class lfo final
{
public:
	lfo(float frequency = 1.0f, float phase = 0.0f);
	void update(double dt);
	auto get() const -> float;

private:
	float m_phase;
	const float m_frequency;
};
} // namespace mark