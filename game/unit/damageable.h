#pragma once
#include "base.h"
#include <interface/damageable.h>

namespace mark {
namespace unit {
class damageable : public unit::base, public interface::damageable {
public:
	virtual auto collide(const segment_t&) -> std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>> = 0;
	virtual auto collide(vector<double> center, float radius)
		-> std::vector<std::reference_wrapper<interface::damageable>> = 0;
	virtual auto radius() const -> double = 0;

protected:
	template <typename... Args>
	inline damageable(Args&&... args)
		: base(std::forward<Args>(args)...)
	{
	}
};
} // namespace unit
} // namespace mark