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

	struct SomeStruct
	{
	private:
		event_binding_container;

	public:
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

		size_t getBoundCount() const
		{
			return eventBindingContainer.size();
		}
	};
}

TEST_CASE("Simple member function event", "[member functions]")
{
	setupWatchers();

	Event<int> event1;

	SomeStruct str;

	event1.bind(str, &SomeStruct::function1);
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Simple member function event with operator", "[member functions]")
{
	setupWatchers();

	Event<int> event1;

	SomeStruct str;

	event1 += event_bind(str, &SomeStruct::function1);
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Multiple callbacks simple member function event", "[member functions]")
{
	setupWatchers();

	Event<int> event1;

	SomeStruct str;

	event1.bind(str, &SomeStruct::function1);
	event1.bind(str, &SomeStruct::function2);
	event1.bind(str, &SomeStruct::function3);
	event1(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(std::get<0>(watcher2->data) == 42 + 42);
	REQUIRE(std::get<0>(watcher3->data) == 42 + 1337);
}

TEST_CASE("Multiple calls member function event", "[member functions]")
{
	setupWatchers();

	Event<int> event1;

	SomeStruct str;

	event1.bind(str, &SomeStruct::function1);
	event1(42);
	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(48);
	REQUIRE(std::get<0>(watcher1->data) == 48);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 1337);
}

TEST_CASE("Scoped member function event", "[member functions]")
{
	setupWatchers();

	Event<int> event1;

	{
		SomeStruct str;
		event1.bind(str, &SomeStruct::function1);
		event1(42);
	}

	REQUIRE(std::get<0>(watcher1->data) == 42);

	event1(1337);
	REQUIRE(std::get<0>(watcher1->data) == 42);
}

TEST_CASE("Cleanup after destruction", "[member functions]")
{
	setupWatchers();

	Event<int> *event1 = new Event<int>();

	SomeStruct str;
	event1->bind(str, &SomeStruct::function1);
	(*event1)(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(str.getBoundCount() == 1);

	// At this point, event's destructor should clean up
	// class' callback bindings
	delete event1;

	REQUIRE(str.getBoundCount() == 0);
}

TEST_CASE("Binding lambda event to class scope", "[member functions]")
{
	setupWatchers();

	Event<int>* event1 = new Event<int>();

	SomeStruct str;
	event1->bind(str, [&str](int x) {
		str.function1(x);
	});
	(*event1)(42);

	REQUIRE(std::get<0>(watcher1->data) == 42);
	REQUIRE(str.getBoundCount() == 1);

	// At this point, event's destructor should clean up
	// class' callback bindings
	delete event1;

	REQUIRE(str.getBoundCount() == 0);
}