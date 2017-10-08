#pragma once
#include "stdafx.h"
#include "unit_base.h"
#include "idamageable.h"

namespace mark {
namespace unit {

class damageable:
	public mark::unit::base,
	public interface::damageable {
public:
	virtual auto collide(const mark::segment_t&) ->
		std::pair<interface::damageable*, mark::vector<double>> = 0;
	virtual auto collide(mark::vector<double> center, float radius) ->
		std::vector<std::reference_wrapper<interface::damageable>> = 0;
protected:
	template<typename...Args>
	inline damageable(Args&&... args)
		: base(std::forward<Args>(args)...) {}
};

} }