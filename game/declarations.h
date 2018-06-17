#pragma once
#include <memory>
// Common forward declarations

namespace sf {
class Color;
}

namespace mark {
template <typename T>
using ref = std::reference_wrapper<T>;
template <typename T>
using cref = ref<const T>;
template <typename T>
using not_null = gsl::not_null<T>;
template <typename T>
using shared_ptr = std::shared_ptr<T>;
template <typename T>
using unique_ptr = std::unique_ptr<T>;
template <typename T>
using weak_ptr = std::weak_ptr<T>;
class map;
class particle;
struct path;
class property_manager;
class property_serializer;
struct rectangle;
struct sprite;
struct window;
using renderable = std::variant<sprite, path, rectangle, window>;
class targeting_system;
struct update_context;
class world;
class world_stack;
namespace interface {
class container;
class damageable;
class has_bindings;
class item;
using item_ptr = unique_ptr<item>;
class world_object;
}; // namespace interface
namespace module {
class base;
using base_ptr = unique_ptr<base>;
class cargo;
class core;
struct modifiers;
} // namespace module
namespace unit {
class base;
class modular;
class landing_pad;
} // namespace unit
namespace resource {
class manager;
class image;
using image_ptr = shared_ptr<const image>;
} // namespace resource
} // namespace mark
