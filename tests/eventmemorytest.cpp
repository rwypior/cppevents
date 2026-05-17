#include "common.h"
#include "events/events.h"

#include <catch2/catch_all.hpp>

#include <iostream>

namespace {
	void function1(int)
	{
		// Whatever
	}

	void function2(int)
	{
		// Whatever
	}

	struct SomeStruct
	{
	private:
		event_binding_container;

	public:
		void function1(int)
		{
			// Whatever
		}

		void function2(int)
		{
			// Whatever
		}
	};
}

TEST_CASE("Free function event copying", "[memory]")
{
	Event<int> event1;

	scoped_event event1 += function1;
	scoped_event event1 += function2;

	Event<int> copy1 = event1;
	REQUIRE(copy1.count() == 2);
}

TEST_CASE("Member function event copying", "[memory]")
{
	SomeStruct something;

	Event<int> event1;

	event1 += event_bind(something, &SomeStruct::function1);
	event1 += event_bind(something, &SomeStruct::function2);

	Event<int> copy1 = event1;
	REQUIRE(copy1.count() == 2);
}

TEST_CASE("Lambda event copying", "[memory]")
{
	SomeStruct something;

	Event<int> event1;

	scoped_event event1 += [](int) { };
	scoped_event event1 += [](int) { };

	Event<int> copy1 = event1;
	REQUIRE(copy1.count() == 2);
}

TEST_CASE("One-time event copying", "[memory]")
{
	SomeStruct something;

	Event<int> event1;

	event1.once([](int) {});
	event1.once([](int) {});

	Event<int> copy1 = event1;
	REQUIRE(copy1.count() == 2);
}

TEST_CASE("Event moving", "[memory]")
{
	Event<int> event1;

	scoped_event event1 += function1;
	scoped_event event1 += function2;
	REQUIRE(event1.count() == 2);

	Event<int> copy1 = std::move(event1);
	REQUIRE(event1.count() == 0);
	REQUIRE(copy1.count() == 2);
}