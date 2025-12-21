#pragma once

#include "include_all.h"

namespace Utilities
{
	template <typename T, typename U>
	bool instanceof(U* toCheck, T*& casted) {
		T* casted = dynamic_cast<T*>(toCheck);
		return (casted != nullptr);
	}
}