#include "stdafx.h"
#include "adsr.h"

namespace mark {

adsr::adsr(float attack, float decay, float sustain, float release)
	: m_attack(attack)
	, m_decay(decay)
	, m_sustain(sustain)
	, m_release(release)
	, m_state(attack + sustain + release)
{
	assert(release > 0);
}

void adsr::trigger() { m_state = 0.f; }

void adsr::update(double dt)
{
	m_state += static_cast<float>(dt);
	if (m_state > m_attack + m_sustain + m_release) {
		m_state = m_attack + m_sustain + m_release;
	}
}

double adsr::get() const
{
	if (m_state < m_attack) {
		return m_state * m_decay / m_attack;
	}
	if (m_state < m_attack + m_sustain) {
		return m_decay;
	}
	if (m_state < m_attack + m_sustain + m_release) {
		return m_decay - (m_state - m_attack - m_sustain) * m_decay / m_release;
	}
	return 0.f;
}
} // namespace mark