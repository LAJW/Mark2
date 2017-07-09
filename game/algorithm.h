#pragma once
#include "stdafx.h"

namespace mark {
	template <typename T>
	class enumerator {
	public:
		class iterator:
			public std::iterator<std::bidirectional_iterator_tag, T, void> {
		public:
			iterator(T value): m_value(value) { }
			auto& operator++() noexcept {
				m_value++;
				return *this;
			}
			auto& operator--() noexcept {
				m_value--;
				return *this;
			}
			auto operator*() const noexcept {
				return m_value;
			}
			auto operator==(const iterator& other) const noexcept {
				return m_value == other.m_value;
			}
			auto operator!=(const iterator& other) const noexcept {
				return m_value != other.m_value;
			}
		private:
			T m_value;
		};
		using const_iterator = iterator;
		enumerator(T begin, T end):
			m_begin(begin),
			m_end(end) { }
		iterator begin() const noexcept {
			return m_begin;
		}
		iterator end() const noexcept {
			return m_end;
		}
	private:
		iterator m_begin;
		iterator m_end;
	};

	template<typename T>
	class area_t {
	public:
		class iterator:
			public std::iterator<
				std::bidirectional_iterator_tag,
				mark::vector<T>, void> {
		public:
			iterator(const mark::area_t<T>& area, mark::vector<T> i):
				m_area(area),
				m_i(i) {}
			iterator& operator++() noexcept {
				m_i.x++;
				if (m_i.x == m_area.bottom_right.x) {
					m_i.x = m_area.top_left.x;
					m_i.y++;
				}
				return *this;
			}
			iterator& operator--() noexcept {
				if (m_i.x > m_area.top_left.x) {
					m_i.x--;
				} else {
					m_i.x = m_area.bottom_right.x - 1;
					m_i.y--;
				}
				return *this;
			}
			auto operator==(const iterator& other) const noexcept {
				return other.m_i == m_i;
			}
			auto operator!=(const iterator& other) const noexcept {
				return other.m_i != m_i;
			}
			auto operator*() const noexcept {
				return m_i;
			}
		private:
			const mark::area_t<T>& m_area;
			mark::vector<T> m_i = 0;
		};
		using const_iterator = iterator;
		area_t(mark::vector<T> bottom_right):
			top_left(0, 0),
			bottom_right(bottom_right) {}
		area_t(mark::vector<T> top_left, mark::vector<T> bottom_right):
			top_left(top_left),
			bottom_right(bottom_right) {}
		const_iterator begin() const noexcept {
			return iterator(*this, top_left);
		}
		const_iterator end() const noexcept {
			return iterator(*this, { top_left.x, bottom_right.y });
		}
		const mark::vector<T> top_left;
		const mark::vector<T> bottom_right;
	};

	template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>> 
	auto enumerate(T min, T max) {
		return mark::enumerator<T>(min, max);
	}

	template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>> 
	auto enumerate(T max) {
		return mark::enumerator<T>(static_cast<T>(0), max);
	}

	template<typename T> 
	auto enumerate(mark::vector<T> min, mark::vector<T> max) {
		return mark::area_t<T>(min, max);
	}

	template<typename T> 
	auto enumerate(mark::vector<T> max) {
		return mark::area_t<T>(max);
	}

	template <typename iterator_t, typename pred_t>
	auto has_if(iterator_t begin, iterator_t end, pred_t& pred) {
		return std::find_if(begin, end, pred) != end;
	}

	template <typename container_t, typename pred_t>
	auto has_if(container_t container, pred_t& pred) {
		const auto begin = std::begin(container);
		const auto end = std::end(container);
		return std::find_if(begin, end, pred) != end;
	}
}