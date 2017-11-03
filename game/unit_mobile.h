#pragma once
#include "unit_damageable.h"

namespace mark {
namespace unit {
class mobile : public damageable {
public:
	void serialise(YAML::Emitter& out) const override;
	virtual auto radius() const -> double = 0;
	void stop();
protected:
	void command(const command::any&) override;
	mobile(mark::world& world, const YAML::Node&);
	template<typename...Ts>
	mobile(Ts&&... ts): damageable(std::forward<Ts>(ts)...) { }
	void tick_movement(double dt, double velocity, bool ai);
private:
	double m_velocity = 0.0;
	vector<double> m_moveto;
	std::vector<vector<double>> m_path_cache;
	float m_path_age = 0.f;
};
}
}