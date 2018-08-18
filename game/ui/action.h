#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {

class action {
public:
	virtual ~action() = default;
	virtual void execute() = 0;
protected:
	action() = default;
};

/// Action spawned from a lambda for backwards-compatibility purposes
/// To be removed once we know what the common actions are and what are their
/// arguments
class legacy_action final : public action {
public:
	explicit legacy_action(const std::function<void()> &callback)
		: m_callback(callback)
	{}
	void execute() override
	{
		m_callback();
	}
private:
	const std::function<void()> m_callback;
	
};

}
}