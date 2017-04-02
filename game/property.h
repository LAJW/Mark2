#pragma once

template <typename T>
class Property {
private:
	T m_val;
public:
	Property(const T& val) : m_val(val) {

	}
	T& operator()() {
		return m_val;
	}
	const T& operator()() const {
		return  m_val;
	};
	void operator()(const T& t) {
		m_val = t;
	};
};