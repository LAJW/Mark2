#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {

class action {
public:
	virtual ~action() = default;
protected:
	action() = default;
};

}
}