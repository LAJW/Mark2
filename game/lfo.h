#pragma once
#include "stdafx.h"

namespace mark {
	class lfo final {
	public:
		lfo(float frequency = 1.0f, float phase = 0.0f);
		void tick(double dt);
		auto get() -> float;
	private:
		float m_phase;
		const float m_frequency;
	};
}