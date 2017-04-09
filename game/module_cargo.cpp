#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager)
	:mark::module::base({ 2, 4 }), m_image(resource_manager.image("cargo.png")) {
}

auto mark::module::cargo::render() const -> std::vector<mark::sprite> {
	auto pos = socket()->relative_pos();
	return {mark::sprite(m_image, pos.x, pos.y, 64.f, socket()->rotation())};
}