#include "world.h"
#include "resource_manager.h"
#include "unit_base.h"
#include "unit_modular.h"
#include "module_cargo.h"
#include "module_core.h"
#include "sprite.h"

mark::world::world(mark::resource::manager& resource_manager)
	:m_resource_manager(resource_manager) {

	auto vessel = std::make_shared<mark::unit::modular>(*this, mark::vector<double>{ 100, 100 }, 10);
	auto core = std::make_unique<mark::module::core>(m_resource_manager);
	vessel->attach(std::move(core), { -1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { 1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { -3, -1 });
	m_units.push_back(vessel);
}


auto mark::world::render() const -> std::vector<mark::sprite> {
	std::vector<mark::sprite> sprites;
	for (auto& unit : m_units) {
		auto socket_sprites = unit->render();
		sprites.insert(
			sprites.end(),
			std::make_move_iterator(socket_sprites.begin()),
			std::make_move_iterator(socket_sprites.end())
		);
	}
	return sprites;
}

auto mark::world::resource_manager() -> mark::resource::manager& {
	return m_resource_manager;
}

void mark::world::tick(double dt) {
	for (auto& unit : m_units) {
		unit->tick(dt);
	}
}