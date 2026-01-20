#ifndef _h_events
#define _h_events

#include <memory>
#include <set>
#include <vector>
#include <functional>

#define __EVT2(A, B, C) A ## B ## C
#define __EVT(A, B) __EVT2(A, B, =)

// Define scoped event callback - used for proper callback destruction when it goes out of scope
#define scoped_event __EVT(auto evt_, __LINE__)

// Define event callback binding container in client class - used for proper callback destruction
// Use at the top of client class. Best if it's private.
#define event_binding_container template<typename ...T> friend class Event; EventBindingContainer eventBindingContainer;

// The very base event callback, used for proper memory management
class CallbackBase
{
public:
	virtual ~CallbackBase() = default;
};

// Keeps track of registered callbacks in order to assure proper callback destruction
class EventBindingContainer
{
public:
	void registerCallback(const std::shared_ptr<CallbackBase> &cb)
	{
		this->eventBindingContainer.insert(cb);
	}

	void unregisterCallback(const std::shared_ptr<CallbackBase> &cb)
	{
		this->eventBindingContainer.erase(cb);
	}

	size_t size() const
	{
		return eventBindingContainer.size();
	}

private:
	std::set<std::shared_ptr<CallbackBase>> eventBindingContainer;
};

// Base event callback
template<typename ...T>
class Callback : public CallbackBase
{
	template<typename ...T> friend class Event;

public:
	virtual ~Callback() = default;
	virtual void operator()(T... args) = 0;

protected:
	virtual void unregister(std::shared_ptr<Callback<T...>> &cb) {}
};

// Event callback functor for free functions
template<typename ...T>
class FreeCallback : public Callback<T...>
{
	template<typename ...T> friend class Event;

public:
	template<typename ...T>
	FreeCallback(const std::function<void(T...)>& cb)
		: callback(cb)
	{
	}

	template<typename ...T>
	FreeCallback(void(func)(T...))
		: callback(func)
	{
	}

	virtual void operator()(T... args) override
	{
		this->callback(args...);
	}

private:
	std::function<void(T...)> callback;
};

// Event callback functor for member functions
template<typename C, typename ...T>
class MemberCallback : public Callback<T...>
{
	template<typename ...T> friend class Event;

public:
	template<typename C, typename ...T>
	MemberCallback(C& c, void(C::* func)(T...))
		: callback(func)
		, cls(c)
	{
	}

	virtual void operator()(T... args) override
	{
		(this->cls.*(this->callback))(args...);
	}

protected:
	virtual void unregister(std::shared_ptr<Callback<T...>>& cb)
	{
		this->cls.eventBindingContainer.unregisterCallback(cb);
	}

private:
	void(C::*callback)(T...);
	C& cls;
};

template<typename ...T>
class Event
{
public:
	using Cb = FreeCallback<T...>;

	~Event()
	{
		this->unregisterMembers();
	}

	// Call the event
	void operator()(T... args) const
	{
		for (auto it = this->callbacks.begin(); it != this->callbacks.end();)
		{
			auto& cb = *it;
			if (auto ptr = cb.lock())
			{
				(*ptr)(args...);
				it++;
			}
			else
				it = this->callbacks.erase(it);
		}
	}

	// Bind a function to this event
	[[nodiscard]]
	std::shared_ptr<Cb> bind(void(cb)(T...))
	{
		return *this += cb;
	}

	// Bind a function to this event
	[[nodiscard]]
	std::shared_ptr<Cb> operator+=(std::function<void(T...)> cb)
	{
		auto ptr = std::make_shared<Cb>(cb);
		this->callbacks.push_back(std::weak_ptr<Cb>(ptr));
		return ptr;
	}

	// Unbind a free callback function from this event by the function address
	void operator-=(void(cb)(T...))
	{
		typedef void(cbtype)(T...);
		auto it = std::find_if(this->callbacks.begin(), this->callbacks.end(), [cb](const auto& a) {
			if (auto f = std::dynamic_pointer_cast<FreeCallback<T...>>(a.lock()))
				return *f->callback.target<cbtype*>() == cb;
			return false;
		});
		if (it != this->callbacks.end())
			this->callbacks.erase(it);
	}

	// Unbind a callback
	void operator-=(const std::shared_ptr<Callback<T...>> &cb)
	{
		auto it = std::find_if(this->callbacks.begin(), this->callbacks.end(), [cb](const auto &a) {
			return cb == a.lock();
		});
		if (it != this->callbacks.end())
			this->callbacks.erase(it);
	}

	// Bind member function to this event
	template<typename C>
	void bind(C& c, void(C::* cb)(T...))
	{
		using Mcb = MemberCallback<C, T...>;
		auto ptr = std::make_shared<Mcb>(c, cb);
		this->callbacks.push_back(std::weak_ptr<Mcb>(ptr));
		c.eventBindingContainer.registerCallback(ptr);
	}

	// Erase all callback references to member callbacks in client classes
	void unregisterMembers()
	{
		for (auto cb : this->callbacks)
		{
			// If the callback is good, then the class object still lives
			if (auto ptr = cb.lock())
				ptr->unregister(ptr);
		}
	}

	// Clear all callbacks
	void clear()
	{
		this->unregisterMembers();
		this->callbacks.clear();
	}

private:
	mutable std::vector<std::weak_ptr<Callback<T...>>> callbacks;
};

#endif