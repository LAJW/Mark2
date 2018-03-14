#include "stdafx.h"
#include "lfo.h"

mark::lfo::lfo(float frequency, float phase)
	: m_frequency(frequency)
	, m_phase(phase)
{
}

void mark::lfo::tick(double dt)
{
	m_phase = std::fmod(m_phase + static_cast<float>(dt), 1 / m_frequency);
}

auto mark::lfo::get() const -> float
{
	return std::sin(m_phase * m_frequency * static_cast<float>(M_PI * 2.0));
}
