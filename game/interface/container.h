#pragma once
#include <exception.h>

namespace mark {
namespace interface {
class container
{
public:
	[[nodiscard]] virtual std::error_code
	attach(const vector<int>& pos, std::unique_ptr<interface::item>& item) = 0;
	virtual auto
	can_attach(const vector<int>& pos, const interface::item& item) const
		-> bool = 0;
	virtual auto at(const vector<int>& pos) const -> const interface::item* = 0;
	virtual auto at(const vector<int>& pos) -> interface::item* = 0;
	virtual auto detach(const vector<int>& pos)
		-> std::unique_ptr<interface::item> = 0;

protected:
	~container() = default;
};
} // namespace interface
} // namespace mark