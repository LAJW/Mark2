#pragma once
#include "stdafx.h"
#include <random.h>

namespace mark {
namespace resource {
class image;

class manager
{
public:
	virtual auto image(const std::string& filename)
		-> resource::image_ptr = 0;
protected:
	random& random;
	manager(mark::random& random);
};

class manager_impl final : public manager
{
public:
	manager_impl();
	auto image(const std::string& filename)
		-> resource::image_ptr override;

private:
	std::unordered_map<std::string, weak_ptr<const resource::image>>
		m_images;
	random_impl m_random;
};

class manager_stub final : public manager
{
public:
	manager_stub();
	auto image(const std::string& filename)
		-> resource::image_ptr override;
private:
	random_stub m_random;
};
}; // namespace resource
} // namespace mark
