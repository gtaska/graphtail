#pragma once

#if defined(_WIN32)
	#include <io.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <signal.h>
	#include <unistd.h>
#endif

#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>