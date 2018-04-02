#pragma once
#include "base.h"

namespace mark {
namespace unit {
// Represents a unit that can be activated by the modular
class activable : public base
{
public:
	[[nodiscard]] virtual auto use(const std::shared_ptr<unit::modular>& by)
		-> std::error_code = 0;

protected:
	template <typename... Ts>
	activable(Ts&&... ts)
		: base(std::forward<Ts>(ts)...)
	{}
};
} // namespace unit
} // namespace mark