#pragma once
#include <stdafx.h>

namespace mark {

// Wrapper around an item and its position in a cargo container or a modular
class slot final
{
public:
	slot() = default;
	slot(interface::container& container, vi32 pos) noexcept;
	auto empty() const noexcept -> bool;
	auto container() noexcept -> interface::container&;
	auto container() const noexcept -> const interface::container&;
	auto pos() const noexcept -> vi32;

private:
	interface::container* m_container = nullptr;
	vi32 m_pos;
};

bool operator==(const slot&, const slot&) noexcept;
bool operator!=(const slot&, const slot&) noexcept;
auto item_of(slot&) noexcept -> interface::item&;
auto item_of(const slot&) noexcept -> const interface::item&;
auto can_detach(const slot&) noexcept -> bool;
auto detach(slot&) noexcept -> interface::item_ptr;

} // namespace mark
