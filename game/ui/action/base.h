#pragma once
#include <stdafx.h>

namespace mark {
class mode_stack;
namespace ui {
namespace action {

class base {
public:
	virtual ~base() = default;
	struct execute_info final {
		optional<mode_stack&> mode_stack;
	};
	virtual void execute(const execute_info& info) = 0;
protected:
	base() = default;
};

}
}
}
