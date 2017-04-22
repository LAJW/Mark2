#pragma once
#include <vector>
#include "iserializable.h"
#include "unit_modular.h"
#include "property.h"
#include "vector.h"
#include "resource_image.h"

namespace mark {
	class sprite;
	namespace unit {
		class modular;
	}

	namespace module {
		static constexpr float size = 16.f;

		// Maximum module width and height
		const unsigned max_dimension = 4;

		class base : public iserializable {
		public:
			base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail);
			virtual ~base() = default;
			inline auto size() const { return m_size; }
			virtual void tick(mark::tick_context& context) = 0;
			virtual auto dead() const -> bool = 0;
			inline auto thumbnail() const { return m_thumbnail; }
			virtual auto detachable() const -> bool { return true; }
			virtual void target(mark::vector<double> pos) { /* no op */ }
			Property<mark::unit::modular::socket*> socket = nullptr;
		private:
			std::shared_ptr<const mark::resource::image> m_thumbnail;
			const mark::vector<unsigned> m_size;
		};

		class energy_generator : public base {
		public:
		private:
		};

		class armor : public base {
		public:
		private:
		};

		class gun : public base {
		public:
		private:
		};

		class laser_gun : public gun {
		public:
		private:
		};

		class rocket_gun : public gun {
		public:
		private:
		};

		class emp_gun : public gun {
		public:
		private:
		};

		class antimatter_gun : public gun {
		public:
		private:
		};

		class flamethrower_gun : public gun {
		public:
		private:
		};

		class radiator : public base {
		public:
		private:
		};

		class ammo : public base {
		public:
		private:
		};

		class drive : public base {
		public:
		private:
		};
	}
}
