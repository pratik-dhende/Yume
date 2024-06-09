#pragma once

#include "Yume/Log.h"

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <type_traits>

#include <comdef.h>
#include <system_error>

#ifdef YM_PLATFORM_WINDOWS
	#include <windows.h>
#endif