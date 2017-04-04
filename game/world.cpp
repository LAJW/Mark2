#include "world.h"
#include "resource_manager.h"
#include "unit_base.h"
#include "unit_modular.h"
#include "module_cargo.h"
#include "module_core.h"
#include "sprite.h"
#include "terrain_floor.h"
#include <random>
#include <array>

static auto make_map(mark::world& world) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor(1000, std::vector<std::shared_ptr<mark::terrain::base>>(1000, nullptr));
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
					floor[cur.x][cur.y] = std::make_shared<mark::terrain::floor>(world);
				}
			}
		}
		point += direction * length;
	}

	return floor;
}

mark::world::world(mark::resource::manager& resource_manager)
	:m_resource_manager(resource_manager) {
	m_map = make_map(*this);

	auto vessel = std::make_shared<mark::unit::modular>(*this, mark::vector<double>{ 0, 0 }, 10.0);
	auto core = std::make_unique<mark::module::core>(m_resource_manager);
	vessel->attach(std::move(core), { -1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { 1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { -3, -1 });
	m_camera_target = vessel;
	m_units.push_back(vessel);

}

auto mark::world::map() const -> const std::vector<std::vector<std::shared_ptr<mark::terrain::base>>>& {
	return m_map;
}

auto mark::world::render() const -> std::vector<mark::sprite> {
	std::vector<mark::sprite> sprites;


	for (int x = 100; x < 800; x++) {
		for (int y = 100; y < 600; y++) {
			const auto& point = m_map[x][y];
			if (point) {
				auto tmp = m_map[x][y]->render({ x - 500, y - 500 });
				sprites.insert(
					sprites.end(),
					std::make_move_iterator(tmp.begin()),
					std::make_move_iterator(tmp.end())
				);
			}
		}
	}

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
	const auto camera_target = m_camera_target.lock();
	if (camera_target) {
		m_camera = camera_target->pos();
	}
}