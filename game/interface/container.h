#pragma once
#include <exception.h>

namespace mark {
namespace module {
class base;
}

namespace interface {
class container {
public:
	[[nodiscard]] virtual std::error_code
	attach(const vector<int>& pos, std::unique_ptr<module::base>& module) = 0;
	virtual auto
	can_attach(const vector<int>& pos, const module::base& module) const
		-> bool = 0;
	virtual auto at(const vector<int>& pos) const -> const module::base* = 0;
	virtual auto at(const vector<int>& pos) -> module::base* = 0;
	virtual auto detach(const vector<int>& pos)
		-> std::unique_ptr<module::base> = 0;

protected:
	~container() = default;
};
} // namespace interface
} // namespace mark