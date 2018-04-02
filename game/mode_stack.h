#pragma once
#include "stdafx.h"

namespace mark {

enum class mode
{
	main_menu,
	world,
	prompt,
};

// Stack of modes - manages in which mode the game should be
class mode_stack final
{
public:
	mode_stack() { m_stack.push_back(mode::main_menu); }
	void push(mode mode)
	{
		if (find(m_stack.begin(), m_stack.end(), mode) == m_stack.end()) {
			m_stack.push_back(mode);
		}
	}
	void pop()
	{
		if (m_stack.size() > 1) {
			m_stack.pop_back();
		} else {
			m_stack.push_back(mode::world);
		}
	}
	auto get() const noexcept -> std::vector<mode> { return m_stack; }
	auto paused() const noexcept -> bool
	{
		return !m_stack.empty() && m_stack.back() != mode::world;
	}
	void clear() { m_stack.clear(); }

private:
	std::vector<mode> m_stack;
};
} // namespace mark
