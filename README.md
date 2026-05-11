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

Reference
---------

### **Event** class
#### template<typename ...T> class Event

#### Template arguments
**...T** - list of parameter types which will be passed to the event's callbacks

#### Member functions
```
void operator()(T... args) const
```

Calls all registered event callbacks with given arguments *args*

-----------

```
[[nodiscard]] std::shared_ptr<Cb> bind(void(cb)(T...))
[[nodiscard]] std::shared_ptr<Cb> operator+=(std::function<void(T...)> cb)
```

Binds a free-function callback to this event. After the function is bound to the event, it will be called once the event is fired.
The callback can be later unregistered by calling *-=* operator. The *scoped_event* macro (explained later) provides automatic
callback unregistration.

-----------

```
template<typename C> void bind(C& c, void(C::* cb)(T...))
template<typename C> void operator+=(EventBinding<C, T...>&& binding)
```

Binds a mamber function callback to this event. For more details, refer to functions related to free-function callbacks.
The *event_bind* and *event_binding_container* macros provide automatic callback unregistration for member-function callbacks
(explained later).

-----------

```
std::shared_ptr<Cb> once(std::function<void(T...)> cb)
```

Registeres one-time only free-function callback to this event.

-----------

```
template<typename C> void once(EventBinding<C, T...>&& binding)
```

Registeres one-time only member-function callback to this event.

-----------

```
void operator-=(void(cb)(T...))
void operator-=(const std::shared_ptr<Callback<T...>> &cb)
```

Unregisteres previously registered callbacks using callback pointer returned from previously used binding functions.

-----------

```
void unregisterMembers()
```

Unregisteres all previously registered **member**-function callback functions from this event.

-----------

```
void clear()
```

Unregisteres **all** previously registered callbacks from this event.

-----------

```
size_t count()
```

Returns count of all registered callbacks to this event.

-----------

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
