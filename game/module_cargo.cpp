#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager)
	:mark::module::base({ 4, 2 }), m_image(resource_manager.image("cargo.png")) {
}

void mark::module::cargo::tick(mark::tick_context& context) {
	auto pos = socket()->relative_pos();
	context.sprites[0].push_back(mark::sprite(m_image, pos.x, pos.y, 64.f, socket()->rotation()));
}