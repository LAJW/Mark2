#include "map.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "vector.h"
#include "sprite.h"

// make_map
#include <random>
#include "terrain_floor.h"
#include "terrain_wall.h"

static auto world_to_map(const mark::vector<double>& pos, const mark::vector<int>& map_size) {
	return mark::vector<int>(std::round(pos.x / 32.0), std::round(pos.y / 32.0)) + map_size / 2;
}

static auto map_to_world(const mark::vector<int>& pos, const mark::vector<int>& map_size) {
	return mark::vector<double>(pos - map_size / 2) * 32.0;
}

static auto make_map(mark::resource::manager& resource_manager) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist_1_100(1, 10);
	std::uniform_int_distribution<> dist_0_3(0, 3);

	std::vector<std::vector<std::shared_ptr<mark::terrain::base>>> floor(1000, std::vector<std::shared_ptr<mark::terrain::base>>(1000, nullptr));
	auto point = mark::vector<int>(500, 500);
	for (int i = 0; i < 100; i++) {
		const auto direction = mark::vector<int>(mark::rotate(mark::vector<float>(1, 0), dist_0_3(gen) * 90.f));
		const auto orto = mark::vector<int>(mark::rotate(mark::vector<float>(direction), 90.f));
		const auto length = dist_1_100(gen);

		for (int j = -5; j < length + 5; j++) {
			const auto step = point + direction * j;
			for (int k = -3; k <= 3; k++) {
				const auto cur = step + orto * k;
				if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000) {
					floor[cur.x][cur.y] = std::make_shared<mark::terrain::floor>(resource_manager);
				}
			}
			{
				const auto cur = step + orto * 4;
				if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000 && !floor[cur.x][cur.y]) {
					floor[cur.x][cur.y] = std::make_shared<mark::terrain::wall>(resource_manager);
				}
			}
			{
				const auto cur = step + orto * -4;
				if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000 && !floor[cur.x][cur.y]) {
					floor[cur.x][cur.y] = std::make_shared<mark::terrain::wall>(resource_manager);
				}
			}
		}
		for (int k = -4; k <= 4; k++) {
			const auto cur = point - direction * 6 + orto * k;
			if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000 && !floor[cur.x][cur.y]) {
				floor[cur.x][cur.y] = std::make_shared<mark::terrain::wall>(resource_manager);
			}
		}
		for (int k = -4; k <= 4; k++) {
			const auto cur = point + direction * (length + 5) + orto * k;
			if (cur.x > 0 && cur.x < 1000 && cur.y > 0 && cur.y < 1000 && !floor[cur.x][cur.y]) {
				floor[cur.x][cur.y] = std::make_shared<mark::terrain::wall>(resource_manager);
			}
		}
		point += direction * length;
	}

	return floor;
}


mark::map::map(mark::resource::manager& resource_manager) {
	m_terrain = make_map(resource_manager);
}

auto mark::map::traversable(mark::vector<double> pos) const -> bool {
	const auto size = this->size();
	return this->traversable(world_to_map(pos, size));
}

auto mark::map::traversable(mark::vector<int> pos) const -> bool {
	const auto size = this->size();
	return pos.x >= 0 && pos.x < size.x
		&& pos.y >= 0 && pos.y < size.y
		&& m_terrain[pos.x][pos.y]
		&& m_terrain[pos.x][pos.y]->traversable();
}

auto mark::map::render(mark::vector<double> world_tl, mark::vector<double> world_br) const->std::vector<mark::sprite> {
	const auto size = this->size();
	const auto tl = world_to_map(world_tl, size);
	const auto br = world_to_map(world_br, size);
	std::vector<mark::sprite> sprites;
	for (int x = std::max(tl.x, 0); x < std::min(size.x, br.x); x++) {
		for (int y = std::max(tl.y, 0); y < std::min(size.y, br.y); y++) {
			const auto& point = m_terrain[x][y];
			if (point) {
				auto tmp = m_terrain[x][y]->render(mark::vector<int>(x, y) - size / 2);
				sprites.insert(
					sprites.end(),
					std::make_move_iterator(tmp.begin()),
					std::make_move_iterator(tmp.end())
				);
			}
		}
	}
	return sprites;
}

auto mark::map::size() const->mark::vector<int> {
	return { static_cast<int>(m_terrain.size()), static_cast<int>(m_terrain[0].size()) };
}

struct Node {
	mark::vector<int> pos;
	int f = 0; // distance from starting + distance from ending (h)
	Node* parent = nullptr;
};

auto mark::map::find_path(mark::vector<double> world_start, mark::vector<double> world_end) const -> std::vector<mark::vector<double>> {
	const auto size = this->size();
	const auto start = world_to_map(world_start, size);
	const auto end = world_to_map(world_end, size);
	std::vector<Node> open = { Node{ start, static_cast<int>(mark::length(end - start)), nullptr } };
	std::vector<std::unique_ptr<Node>> closed;

	while (!open.empty()) {
		auto min_it = open.begin();
		for (auto it = open.begin(), end = open.end(); it != end; it++) {
			if (min_it->f > it->f) {
				min_it = it;
			}
		}
		closed.push_back(std::make_unique<Node>(*min_it));
		auto& current = closed.back();
		open.erase(min_it);

		if (current->pos == end) {
			std::vector<mark::vector<double>> out;
			auto node = current.get();
			while (node) {
				out.push_back(map_to_world(node->pos, size));
				node = node->parent;
			}
			return out;
		}

		for (int i = 0; i < 9; i++) {
			if (i == 4) {
				continue;
			}
			auto neighbour_pos = current->pos + mark::vector<int>{ i % 3 - 1, i / 3 - 1 };
			const auto traversable = m_terrain[neighbour_pos.x][neighbour_pos.y] && m_terrain[neighbour_pos.x][neighbour_pos.y]->traversable();
			const auto isClosed = closed.end() != std::find_if(closed.begin(), closed.end(), [&neighbour_pos](std::unique_ptr<Node>& node) {
				return node->pos == neighbour_pos;
			});
			if (!traversable || isClosed) {
				continue;
			}
			auto neighbour_it = std::find_if(open.begin(), open.end(), [&neighbour_pos](const Node& node) {
				return neighbour_pos == node.pos;
			});
			const auto f = current->f + (i % 2 ? 10 : 14);
			if (neighbour_it == open.end()) {
				open.push_back({ neighbour_pos, f, current.get() });
			} else if (neighbour_it->f > f) {
				neighbour_it->f = f;
				neighbour_it->parent = current.get();
			}
		}
	}
	return std::vector<mark::vector<double>>();
}