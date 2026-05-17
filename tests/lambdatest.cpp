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
}

TEST_CASE("Lambda::Simple event", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += [](int data) {
		(*watcher1)(data);
	};
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Lambda::Multiple callbacks", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += [](int data) {
		(*watcher1)(data);
	};
	scoped_event event1 += [](int data) {
		(*watcher2)(data + 42);
	};
	scoped_event event1 += [](int data) {
		(*watcher3)(data + 1337);
	};
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
	REQUIRE(std::get<0>(watcher3->data) == 42 + 1337);
}

TEST_CASE("Lambda::Multiple calls", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	scoped_event event1 += [](int data) {
		(*watcher1)(data);
	};
	event1(42);
	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(48);
	REQUIRE(std::get<0>(watcher1->data) == 48);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 1337);
}

TEST_CASE("Lambda::Scoped event", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	{
		scoped_event event1 += [](int data) {
			(*watcher1)(data);
		};
		event1(42);
	}

	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Lambda::Unbinding callbacks", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	auto cb1 = event1 += [](int data) {
		(*watcher1)(data);
	};
	auto cb2 = event1 += [](int data) {
		(*watcher2)(data + 42);
	};

	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);

	event1 -= cb2;

	event1(0);

	REQUIRE(std::get<0>(watcher1->data) == 0);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
}

TEST_CASE("Lambda::Explicit std function", "[lambda]")
{
	setupWatchers();

	Event<int> event1;

	std::function<void(int)> func = [](int data) {
		(*watcher1)(data);
	};

	scoped_event event1 += func;

	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
}