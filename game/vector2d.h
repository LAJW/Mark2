#pragma once
#include <stdafx.h>

namespace mark {
template <typename T>
class vector2d final
{
private:
	using data_type = std::vector<T>;
	unique_ptr<data_type> m_data;
	vector<size_t> m_size;

	template <typename U>
	static auto at(U& self, const vector<size_t>& pos)
	{
		let& size = self.m_size;
		Expects(pos.x < size.x);
		Expects(pos.y < size.y);
		return std::ref((*self.m_data)[pos.x % size.x + pos.y * size.y]);
	}

public:
	vector2d() = default;
	vector2d(vector<size_t> size)
		: m_data(std::make_unique<data_type>(size.x * size.y))
		, m_size(size)
	{}
	auto operator[](const vector<size_t>& pos) -> T& { return at(*this, pos); }
	auto operator[](const vector<size_t>& pos) const -> const T&
	{
		return at(*this, pos).get();
	}
	auto data() -> data_type& { return *m_data; }
	auto data() const -> const data_type& { return *m_data; }
	auto size() const -> const vector<size_t>& { return m_size; }
	auto empty() const { return m_data.empty(); }
	void fill(const T& t) { m_data->fill(t); }
};
} // namespace mark
