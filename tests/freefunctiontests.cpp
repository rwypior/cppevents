#include "common.h"
#include "events/events.h"

#include <catch2/catch_all.hpp>

#include <iostream>

namespace
{
	std::unique_ptr<Watcher<int>> watcher1;
	std::unique_ptr<Watcher<int>> watcher2;
	std::unique_ptr<Watcher<int>> watcher3;

	void setupWatchers()
	{
		watcher1 = std::make_unique<Watcher<int>>();
		watcher2 = std::make_unique<Watcher<int>>();
		watcher3 = std::make_unique<Watcher<int>>();
	}

	void function1(int data)
	{
		(*watcher1)(data);
	}

	void function2(int data)
	{
		(*watcher2)(data + 42);
	}

	void function3(int data)
	{
		(*watcher3)(data + 1337);
	}
}

TEST_CASE("Simple free function event", "[free functions]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += function1;
	event1(42);
	
	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Multiple callbacks simple free function event", "[free functions]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += function1;
	scoped_event event1 += function2;
	scoped_event event1 += function3;
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
	REQUIRE(std::get<0>(watcher3->data) == 42 + 1337);
}

TEST_CASE("Multiple calls free function event", "[free functions]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += function1;
	event1(42);
	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(48);
	REQUIRE(std::get<0>(watcher1->data) == 48);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 1337);
}

TEST_CASE("Scoped free function event", "[free functions]")
{
	setupWatchers();

	Event<int> event1;

	{
		scoped_event event1 += function1;
		event1(42);
	}

	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Unbinding free callbacks", "[free functions]")
{
	setupWatchers();

	Event<int> event1;

	auto cb1 = event1 += function1;
	auto cb2 = event1 += function2;

	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);

	event1 -= cb2;

	event1(0);

	REQUIRE(std::get<0>(watcher1->data) == 0);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
}

TEST_CASE("Unbinding free functions", "[free functions]")
{
	//std::shared_ptr<Callback<int>> asdasd;
	//std::shared_ptr<FreeCallback<int>> zxczxc = std::dynamic_pointer_cast<FreeCallback<int>;

	setupWatchers();

	Event<int> event1;

	scoped_event event1 += function1;
	scoped_event event1 += function2;

	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);

	event1 -= function2;

	event1(0);

	REQUIRE(std::get<0>(watcher1->data) == 0);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
}