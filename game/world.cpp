#include "world.h"
#include "resource_manager.h"
#include "unit_base.h"
#include "unit_modular.h"
#include "module_cargo.h"
#include "module_core.h"
#include "sprite.h"
#include "wall.h"
#include <random>
#include <array>

static auto make_floor(mark::world& world) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	std::vector<std::vector<int>> floor(1000, std::vector<int>(1000, 0));
	auto point = mark::vector<int>(500, 500);
	for (int i = 0; i < 10; i++) {
		const auto direction = mark::vector<int>(mark::rotate(mark::vector<float>(1, 0), dist_0_3(gen) * 90.f));
		const auto orto = mark::vector<int>(mark::rotate(mark::vector<float>(direction), 90.f));
		const auto length = dist_1_100(gen);
		
		for (int j = -5; j < length + 5; j++) {
			const auto step = point + direction * j;
			for (int k = -3; k < 3; k++) {
				const auto cur = step + orto * k;
				if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000) {
					floor[cur.x][cur.y] = 1;
				}
			}
		}
		point += direction * length;
	}
	
	std::vector<std::shared_ptr<mark::unit::base>> walls;
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (floor[x][y] == 1) {
				walls.emplace_back(std::make_shared<mark::unit::wall>(world, mark::vector<double>((x - 500) * 32.0 + 300.0, (y - 500) * 32.0 + 300.0)));
			}
		}
	}

	return walls;
}


mark::world::world(mark::resource::manager& resource_manager)
	:m_resource_manager(resource_manager) {


	const auto corner = make_floor(*this);
	m_units.insert(m_units.end(), corner.begin(), corner.end());

	auto vessel = std::make_shared<mark::unit::modular>(*this, mark::vector<double>{ 300.0, 300.0 }, 10.0);
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