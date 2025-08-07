C++ event library
=================

This project provides a simple, header-only library which adds basic event functionality 
for any C++ function, lambda or class without the need of inheriting any special classes.

Installation
------------

To start using this library, use either of the following methods:

1. Use CMake and `target_link_libraries` to `events::events` target
2. Copy events.h file and include it in your project

Usage
-----

The only class the user has to interact with is the `Event` template class, which creates
an event to which any number of functions can be bound. It can be used with free-functions,
lambda functions and member functions. Here are some examples.

```
#include <events/events.h>

void myFunction(int arg)
{
	std::cout << "myFunction called by event with argument " << arg << std::endl;
}

struct SomeStruct
{
	event_binding_container;

	void fnc(int arg)
	{
		std::cout << "someStruct::fnc called by event with argument " << arg << std::endl;
	}
}

int main(int argc, char **argv)
{
	Event<int> myEvent;

	scoped_event myEvent += myFunction;
	scoped_event myEvent += [](int arg) {
		std::cout << "Lambda called by event with argument " << arg << std::endl;
	};

	SomeStruct str;
	myEvent.bind(str, &SomeStruct::fnc);

	myEvent(42);
}
```

Important macros
----------------

### scoped_event

This macro provides unique name for event callback. This assures proper callback destruction
when it goes out of scope. It should always be used for free-functions and lambda functions
if you don't care about the callback handle.

Callback handle may come in handy especially for lambda functions, as it's the only way to
unregister a lambda callback. In order to do so, you should capture the callback, and unregister it,
like so:

```
auto callback = myEvent += [](){ ... }; // Register lambda
myEvent -= callback; // Unregister lambda
```

If the callback is neither captured into a variable, or by `scoped_event` macro, then memory corruption
may happen when the callback goes out of scope.

### event_binding_container

This macro is required for classes which may contain event handlers. It provides similar role as
`scoped_event` macro, but in this case it creates a container, which stores all callbacks in the class
so they can be unregistered and destroyed properly after the class is destroyed.

If your class has any member which is an event callback, simply put `event_binding_container` at the top
of the class and that's it.

More examples
-------------

For more examples, please refer to tests.