#pragma once
#include <memory>
#include <array>

namespace mark {
namespace resource {
class image;
}
namespace module {
class base;
}

namespace interface {

class has_bindings {
public:
	struct binding final {
		std::shared_ptr<const resource::image> thumbnail;
		uint16_t total = 0;
		uint16_t ready = 0;
		float cooldown = 0.f;
		std::vector<std::reference_wrapper<const module::base>> modules;
	};
	constexpr static const size_t binding_count = 11;
	using bindings_t = std::array<binding, binding_count>;
	virtual auto bindings() const->bindings_t = 0;
protected:
	virtual ~has_bindings() = default;
};

}
}

