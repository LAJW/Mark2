#include "cooldown.h"
#include <algorithm>

mark::cooldown::cooldown(float max):
	m_max(max) {}

void mark::cooldown::tick(double dt) {
	m_now = std::max(m_now - static_cast<float>(dt), 0.f);
}

bool mark::cooldown::ready() {
	return m_now == 0.f;
}

bool mark::cooldown::trigger() {
	const auto ready = this->ready();
	if (ready) {
		m_now = m_max;
	}
	return ready;
}