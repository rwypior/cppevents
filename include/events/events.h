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
#define event_binding_container \
	template<typename ...T> friend class Event; \
	template<typename C, typename ...T> friend class ScopedCallback; \
	template<typename C, typename ...T> friend class MemberCallback; \
	EventBindingContainer eventBindingContainer;

#define event_bind(CLS, FNC) EventBinding(CLS, FNC)

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

// Event callback functor for free, but scope-bound callbacks
template<typename C, typename ...T>
class ScopedCallback : public Callback<T...>
{
	template<typename ...T> friend class Event;

public:
	template<typename ...T>
	ScopedCallback(C& c, const std::function<void(T...)>& cb)
		: callback(cb)
		, cls(c)
	{
	}

	template<typename ...T>
	ScopedCallback(C& c, void(func)(T...))
		: callback(func)
		, cls(c)
	{
	}

	virtual void operator()(T... args) override
	{
		this->callback(args...);
	}

protected:
	virtual void unregister(std::shared_ptr<Callback<T...>>& cb)
	{
		this->cls.eventBindingContainer.unregisterCallback(cb);
	}

private:
	std::function<void(T...)> callback;
	C& cls;
};

template<typename C, typename ...T>
struct EventBinding
{
	using Mcb = MemberCallback<C, T...>;
	std::shared_ptr<Mcb> ptr;
	C& c;

	EventBinding(C& c, void(C::* cb)(T...))
		: ptr(std::make_shared<Mcb>(c, cb))
		, c(c)
	{
	}

	EventBinding(C& c, std::function<void(T...)> cb)
		: ptr(std::make_shared<Mcb>(c, cb))
		, c(c)
	{
	}
};

template<typename ...T>
class Event
{
private:
	// Wrapper for registerd callbacks
	struct EventCallbackStorage
	{
		/// Run the callback and return true to keep it, and false to remove it
		virtual bool run(T... args) const = 0;
		/// Unregister the callback
		virtual void unregister() = 0;
		/// Get callback underlying pointer
		virtual std::shared_ptr<Callback<T...>> get() = 0;
		/// Get callback underlying pointer - const
		virtual const std::shared_ptr<Callback<T...>> get() const = 0;
	};

	// Wrapper for all regular callbacks
	struct StandardEventCallbackStorage : EventCallbackStorage
	{
		StandardEventCallbackStorage(std::weak_ptr<Callback<T...>>&& callback)
			: callback(std::move(callback))
		{
		}

		virtual bool run(T... args) const override
		{
			if (auto ptr = this->callback.lock())
			{
				(*ptr)(args...);
				return true;
			}

			return false;
		}

		virtual void unregister() override
		{
			// If the callback is good, then the class object still lives
			if (auto ptr = this->callback.lock())
				ptr->unregister(ptr);
		}

		virtual std::shared_ptr<Callback<T...>> get() override
		{
			return this->callback.lock();
		}
		
		virtual const std::shared_ptr<Callback<T...>> get() const override
		{
			return this->callback.lock();
		}

		std::weak_ptr<Callback<T...>> callback;
	};

	// Wrapper for one-time callbacks which are deleted after the first call
	struct OneTimeEventCallbackStorage : EventCallbackStorage
	{
		OneTimeEventCallbackStorage(std::shared_ptr<Callback<T...>>&& callback)
			: callback(std::move(callback))
		{
		}

		virtual bool run(T... args) const override
		{
			(*this->callback)(args...);
			return false;
		}

		virtual void unregister() override
		{
			this->callback->unregister(this->callback);
		}

		virtual std::shared_ptr<Callback<T...>> get() override
		{
			return this->callback;
		}
		
		virtual const std::shared_ptr<Callback<T...>> get() const override
		{
			return this->callback;
		}

		std::shared_ptr<Callback<T...>> callback;
	};

	// Wrapper for member one-time callbacks which are deleted after the first call
	struct OneTimeMemberEventCallbackStorage : StandardEventCallbackStorage
	{
		using StandardEventCallbackStorage::StandardEventCallbackStorage;

		virtual bool run(T... args) const override
		{
			if (auto ptr = this->callback.lock())
			{
				(*ptr)(args...);
			}

			return false;
		}
	};

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
			if (cb->run(args...))
				it++;
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

	// Bind a one-time free function to this event
	std::shared_ptr<Cb> once(std::function<void(T...)> cb)
	{
		auto ptr = std::make_shared<Cb>(cb);
		this->callbacks.push_back(std::make_unique<OneTimeEventCallbackStorage>(ptr));
		return ptr;
	}

	// Bind a one-time member function to this event
	template<typename C>
	void once(EventBinding<C, T...>&& binding)
	{
		using Mcb = MemberCallback<C, T...>;
		this->callbacks.push_back(std::make_unique<OneTimeMemberEventCallbackStorage>(std::weak_ptr<Mcb>(binding.ptr)));
		binding.c.eventBindingContainer.registerCallback(binding.ptr);
	}

	// Bind a function to this event
	[[nodiscard]]
	std::shared_ptr<Cb> operator+=(std::function<void(T...)> cb)
	{
		auto ptr = std::make_shared<Cb>(cb);
		this->callbacks.push_back(std::make_unique<StandardEventCallbackStorage>(std::weak_ptr<Cb>(ptr)));
		return ptr;
	}

	// Bind a member-function to this event
	template<typename C>
	void operator+=(EventBinding<C, T...>&& binding)
	{
		using Mcb = MemberCallback<C, T...>;
		this->callbacks.push_back(std::make_unique<StandardEventCallbackStorage>(std::weak_ptr<Mcb>(binding.ptr)));
		binding.c.eventBindingContainer.registerCallback(binding.ptr);
	}

	// Unbind a free callback function from this event by the function address
	void operator-=(void(cb)(T...))
	{
		typedef void(cbtype)(T...);
		auto it = std::find_if(this->callbacks.begin(), this->callbacks.end(), [cb](const auto& a) {
			if (auto f = std::dynamic_pointer_cast<FreeCallback<T...>>(a->get()))
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
			return cb == a->get();
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
		this->callbacks.push_back(std::make_unique<StandardEventCallbackStorage>(std::weak_ptr<Mcb>(ptr)));
		c.eventBindingContainer.registerCallback(ptr);
	}

	// Bind lambda function to this event, with lifetime bound to given class
	template<typename C>
	void bind(C& c, std::function<void(T...)> cb)
	{
		using Mcb = ScopedCallback<C, T...>;
		auto ptr = std::make_shared<Mcb>(c, cb);
		this->callbacks.push_back(std::make_unique<StandardEventCallbackStorage>(std::weak_ptr<Mcb>(ptr)));
		c.eventBindingContainer.registerCallback(ptr);
	}

	// Erase all callback references to member callbacks in client classes
	void unregisterMembers()
	{
		for (auto& cb : this->callbacks)
		{
			cb->unregister();
		}
	}

	// Clear all callbacks
	void clear()
	{
		this->unregisterMembers();
		this->callbacks.clear();
	}

	// Return count of registerd callbacks
	size_t count()
	{
		return this->callbacks.size();
	}

private:
	mutable std::vector<std::unique_ptr<EventCallbackStorage>> callbacks;
};

#endif