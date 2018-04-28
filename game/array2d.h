#pragma once
#include <stdafx.h>

namespace mark {
template <typename T, size_t size_x, size_t size_y>
class array2d final
{
private:
	using data_type = std::array<T, size_x * size_y>;
	unique_ptr<data_type> m_data;

	template <typename U>
	static auto at(U& self, const vector<size_t>& pos)
	{
		Expects(pos.x < size_x);
		Expects(pos.y < size_y);
		return std::ref((*self.m_data)[pos.x % size_x + pos.y * size_y]);
	}

public:
	array2d()
		: m_data(std::make_unique<data_type>())
	{}
	auto operator[](const vector<size_t>& pos) -> T& { return at(*this, pos); }
	auto operator[](const vector<size_t>& pos) const -> const T&
	{
		return at(*this, pos).get();
	}
	auto data() const -> const data_type& { return *m_data; }
	auto size() const -> vector<size_t> { return { size_x, size_y }; }
	void fill(const T& t) { m_data->fill(t); }
};
} // namespace mark
