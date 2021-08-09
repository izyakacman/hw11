#pragma once

#include <string>
#include <vector>
#include <functional>

using MapFunc = std::function<std::string(std::string, size_t)>;
using ReduceFunc = std::function<bool(std::vector<std::string>)>;

bool MapReduce(const std::string& file_name, const std::vector<std::pair<size_t, size_t>> ranges, 
	size_t prefix_count, int rmun, MapFunc map_func, ReduceFunc reduce_func);

