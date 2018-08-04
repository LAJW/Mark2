#pragma once
#include "stdafx.h"

namespace mark {
namespace resource {
class image;

class manager
{
public:
	virtual auto image(const std::string& filename)
		-> resource::image_ptr = 0;
};

class manager_impl final : public manager
{
public:
	auto image(const std::string& filename)
		-> resource::image_ptr override;

private:
	std::unordered_map<std::string, weak_ptr<const resource::image>>
		m_images;
};

class manager_stub final : public manager
{
public:
	auto image(const std::string& filename)
		-> resource::image_ptr override;
};
}; // namespace resource
} // namespace mark
