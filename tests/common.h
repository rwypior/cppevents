#ifndef _h_common
#define _h_common

#include <tuple>

template<typename ...T>
struct Watcher
{
	std::tuple<T...> data;

	void operator()(T... data)
	{
		this->data = { data... };
	}
};

#endif