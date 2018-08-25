#pragma once
#include <stdafx.h>

namespace mark {
namespace interface {
/// UI Public interface
/// Used exclusively by UI and for stubbing UI for testing containers
class ui
{
public:
	[[nodiscard]] virtual optional<const interface::item&> grabbed() const
		noexcept = 0;
	[[nodiscard]] virtual bool
	in_recycler(const mark::interface::item& item) const noexcept = 0;

protected:
	~ui() = default;
};
} // namespace interface
} // namespace mark
