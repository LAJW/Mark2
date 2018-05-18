#pragma once
#include <exception.h>

namespace mark {
namespace interface {
class container
{
public:
	[[nodiscard]] virtual std::error_code
	attach(vi32 pos, interface::item_ptr& item) = 0;
	virtual auto can_attach(vi32 pos, const interface::item& item) const
		-> bool = 0;
	virtual auto at(vi32 pos) const -> const interface::item* = 0;
	virtual auto at(vi32 pos) -> interface::item* = 0;
	// Position at which the item is attached (the top left corner of an item)
	virtual auto pos_at(vi32 pos) const noexcept -> std::optional<vi32> = 0;
	virtual auto detach(vi32 pos) -> interface::item_ptr = 0;
	virtual auto can_detach(vi32 pos) const noexcept -> bool = 0;

protected:
	~container() = default;
};
} // namespace interface
} // namespace mark
