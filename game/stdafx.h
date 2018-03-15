#pragma once
// The following macro has been defined, because yaml-cpp doesn't seem to want
// to compile correctly because it violates C++17 deprecation warnings:
// _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#define _USE_MATH_DEFINES
#define let const auto
#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "declarations.h"
#include "vector.h"
#include "yaml.h"
