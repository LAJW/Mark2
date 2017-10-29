#pragma once
#include "unit_base.h"

namespace mark {
namespace unit {

class modular;
// Represents a unit that can be activated by the modular
class activable : public base {
public:
	[[nodiscard]] virtual auto activate(
		const std::shared_ptr<unit::modular>& by) -> std::error_code = 0;
protected:
	template<typename... Ts>
	activable(Ts&&... ts) : base(std::forward<Ts>(ts)...) { }
};
}
}