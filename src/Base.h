#pragma once

#if defined(_WIN32)
	#include <io.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>