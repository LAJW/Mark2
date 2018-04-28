#pragma once
#include <exception.h>

namespace mark {
namespace interface {
class container
{
public:
	[[nodiscard]] virtual std::error_code
	attach(const vi32& pos, interface::item_ptr& item) = 0;
	virtual auto can_attach(const vi32& pos, const interface::item& item) const
		-> bool = 0;
	virtual auto at(const vi32& pos) const -> const interface::item* = 0;
	virtual auto at(const vi32& pos) -> interface::item* = 0;
	virtual auto detach(const vi32& pos) -> interface::item_ptr = 0;

protected:
	~container() = default;
};
} // namespace interface
} // namespace mark
