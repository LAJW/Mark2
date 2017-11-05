#pragma once
#include "stdafx.h"

namespace mark {
	class adsr {
	public:
		adsr(float attack, float decay, float sustain, float release);
		void trigger();
		void tick(double dt);
		double get() const;
	private:
		const float m_attack;
		const float m_decay;
		const float m_sustain;
		const float m_release;
		float m_state;
	};
}