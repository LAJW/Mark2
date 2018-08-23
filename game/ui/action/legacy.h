#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

/// Action spawned from a lambda for backwards-compatibility purposes
/// To be removed once we know what the common actions are and what are their
/// arguments
class legacy final : public base
{
public:
	explicit legacy(const std::function<void()>& callback)
		: m_void_callback(callback)
	{}
	explicit legacy(const std::function<void(const execute_info&)>& callback)
		: m_callback(callback)
	{}
	void execute(const execute_info& info) override
	{
		if (m_callback) {
			m_callback(info);
		} else {
			m_void_callback();
		}
	}

private:
	const std::function<void()> m_void_callback;
	const std::function<void(const execute_info& info)> m_callback;
};

} // namespace action
} // namespace ui
} // namespace mark
