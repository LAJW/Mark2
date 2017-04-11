#include "adsr.h"

mark::adsr::adsr(float attack, float decay, float sustain, float release):
	m_attack(attack),
	m_decay(decay),
	m_sustain(sustain),
	m_release(release),
	m_state(attack + sustain + release) {
	assert(attack > 0);
	assert(release > 0);
}

void mark::adsr::trigger() {
	m_state = 0.f;
}

void mark::adsr::tick(double dt) {
	m_state += dt;
	if (m_state > m_attack + m_sustain + m_release) {
		m_state = m_attack + m_sustain + m_release;
	}
}

double mark::adsr::get() const {
	if (m_state < m_attack) {
		return m_state * m_decay / m_attack;
	} else if (m_state < m_attack + m_sustain) {
		return m_decay;
	} else if (m_state < m_attack + m_sustain + m_release) {
		return m_decay - m_state * m_decay / m_release;
	} else {
		return 0;
	}
}
