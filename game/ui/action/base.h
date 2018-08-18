#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
namespace action {

class base {
public:
	virtual ~base() = default;
	virtual void execute() = 0;
protected:
	base() = default;
};

}
}
}
