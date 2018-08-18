#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

/// Action spawned from a lambda for backwards-compatibility purposes
/// To be removed once we know what the common actions are and what are their
/// arguments
class legacy final : public base {
public:
	explicit legacy(const std::function<void()> &callback)
		: m_callback(callback)
	{}
	void execute(const execute_info&) override
	{
		m_callback();
	}
private:
	const std::function<void()> m_callback;
};

}
}
}
