#include <catch.hpp>
#include <optional.h>
#include <stdafx.h>

SCENARIO("optional")
{
	using namespace mark;
	GIVEN("optional reference created using its default constructor")
	{
		optional<int&> opt;
		WHEN("We call has_value() on it")
		{
			let result = opt.has_value();
			THEN("It should return false") { REQUIRE(result == false); }
		}
		WHEN("We cast it to bool")
		{
			let result = static_cast<bool>(opt);
			THEN("It should yield false") { REQUIRE(result == false); }
		}
		WHEN("We call .value() on it")
		{
			THEN("It should throw a std::bad_optional_access exception")
			{
				try {
					let result = opt.value();
					REQUIRE(false);
				} catch (const std::bad_optional_access&) {
					REQUIRE(true);
				}
			}
		}
		WHEN("We call reset on it")
		{
			opt.reset();
			THEN("It should remain empty")
			{
				REQUIRE(opt.has_value() == false);
				REQUIRE(!opt);
			}
		}
	}
	GIVEN("optional reference created from a local integer")
	{
		int integer = 42;
		optional<int&> opt = integer;
		WHEN("We call has_value() ")
		{
			let result = opt.has_value();
			THEN("It should return true") { REQUIRE(result == true); }
		}
		WHEN("We cast it to bool")
		{
			let result = static_cast<bool>(opt);
			THEN("It should yield true") { REQUIRE(result == true); }
		}
		WHEN("We call .value() on it")
		{
			let& result = opt.value();
			THEN("The value should be equal to the original integer")
			{
				REQUIRE(result == integer);
			}
			THEN("The value should be reference equal to the integer")
			{
				REQUIRE(&result == &integer);
			}
		}
		WHEN("We dereference it")
		{
			let& result = *opt;
			THEN("The value should be equal to the original integer")
			{
				REQUIRE(result == integer);
			}
			THEN("The value should be reference equal to the integer")
			{
				REQUIRE(&result == &integer);
			}
		}
		WHEN("We call reset on it")
		{
			opt.reset();
			THEN("The optional should be left empty")
			{
				REQUIRE(opt.has_value() == false);
				REQUIRE(!opt);
			}
		}
	}
	GIVEN("optional reference created from a child class")
	{
		struct A
		{
			bool is_const() const { return true; }
			bool is_const() { return false; }
		};
		struct B : A
		{};
		B b;
		optional<B&> opt_b = b;
		WHEN("creating another optional pointing to the base class")
		{
			optional<A&> opt_a = opt_b;
			THEN("both optionals should point to the same object")
			{
				REQUIRE(&opt_a.value() == &opt_b.value());
			}
			THEN("both optionals should not be yield mutable references upon "
				 "dereferencing")
			{
				REQUIRE((*opt_a).is_const() == false);
				REQUIRE((*opt_b).is_const() == false);
			}
			THEN("both optionals should not yield mutable references upon "
				 "calling .value()")
			{
				REQUIRE(opt_a.value().is_const() == false);
				REQUIRE(opt_a.value().is_const() == false);
			}
			THEN("both optionals should not yield mutable references when "
				 "using the arrow operator")
			{
				REQUIRE(opt_a.value().is_const() == false);
				REQUIRE(opt_a.value().is_const() == false);
			}
			THEN("It should be possible to reassign with casting")
			{
				opt_a = opt_b;
				REQUIRE(&opt_a.value() == &opt_b.value());
			}
		}
		WHEN("creating another optional pointing to a const value of B")
		{
			optional<const B&> const_b = opt_b;
			THEN("both optionals should point to the same object")
			{
				REQUIRE(&const_b.value() == &opt_b.value());
			}
			THEN("dereferencing should return a const reference")
			{
				REQUIRE((*const_b).is_const() == true);
			}
			THEN("calling .value() should return a const reference")
			{
				REQUIRE(const_b.value().is_const() == true);
			}
			THEN("using -> operator should yield const")
			{
				REQUIRE(const_b->is_const() == true);
			}
		}
	}
}
