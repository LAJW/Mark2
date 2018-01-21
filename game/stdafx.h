#pragma once
// The following macro has been defined, because yaml-cpp doesn't seem to want
// to compile correctly because it violates C++17 deprecation warnings:
// _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 
#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <assert.h>
#include <cmath>
#include <deque>
#include <functional>
#include <math.h>
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
#include <SFML/Graphics/Color.hpp>
#include "vector.h"
#include "yaml.h"
#include "declarations.h"