#pragma once
#include <vector>
#include "iserializable.h"
#include "unit_modular.h"
#include "property.h"
#include "vector.h"

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
			base(mark::vector<unsigned> size);
			virtual ~base() = default;
			inline auto size() const { return m_size; }
			virtual auto render() const->std::vector<mark::sprite> = 0;
			Property<mark::unit::modular::socket*> socket = nullptr;
		private:
			mark::vector<unsigned> m_size;
		};

		class energy_generator : public base {
		public:
		private:
		};

		class shield_generator : public base {
		public:
		private:
		};

		class armor : public base {
		public:
		private:
		};

		class turret : public base {
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

		class laser_turret : public turret {
		public:
		private:
		};

		class rocket_gun : public gun {
		public:
		private:
		};

		class rocket_turret : public turret {
		public:
		private:
		};

		class emp_gun : public gun {
		public:
		private:
		};

		class emp_turret : public turret {
		public:
		private:
		};

		class antimatter_gun : public gun {
		public:
		private:
		};

		class antimatter_turret : public turret {
		public:
		private:
		};

		class flamethrower_gun : public gun {
		public:
		private:
		};

		class flamethrower_turret : public turret {
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
