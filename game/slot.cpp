#include "slot.h"
#include <interface/container.h>
#include <interface/item.h>

mark::slot::slot(interface::container& container, vi32 pos) noexcept
	: m_container(&container)
	, m_pos(pos)
{
	Expects(container.at(pos).has_value());
}

auto mark::slot::empty() const noexcept -> bool
{
	return m_container == nullptr;
}

auto mark::slot::container() noexcept -> interface::container&
{
	Expects(!empty());
	return *m_container;
}

auto mark::slot::container() const noexcept -> const interface::container&
{
	Expects(!empty());
	return *m_container;
}

auto mark::slot::pos() const noexcept -> vi32
{
	Expects(!empty());
	return m_pos;
}

auto mark::item_of(mark::slot& slot) noexcept -> interface::item&
{
	return *slot.container().at(slot.pos());
}

auto mark::item_of(const mark::slot& slot) noexcept -> const interface::item&
{
	return *slot.container().at(slot.pos());
}

auto mark::can_detach(const mark::slot& slot) noexcept -> bool
{
	return slot.container().can_detach(slot.pos());
}

auto mark::detach(mark::slot& slot) noexcept -> interface::item_ptr
{
	auto item = slot.container().detach(slot.pos());
	slot = {};
	return item;
}

bool mark::operator==(const mark::slot& a, const mark::slot& b) noexcept
{
	if (a.empty() && b.empty()) {
		return true;
	}
	if (a.empty() ^ b.empty()) {
		return false;
	}
	return &a.container() == &b.container() && a.pos() == b.pos();
}

bool mark::operator!=(const mark::slot& a, const mark::slot& b) noexcept
{
	return !(a == b);
}
