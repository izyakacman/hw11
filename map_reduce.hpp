#pragma once

#include <string>
#include <vector>
#include <functional>

using MapReduceFunc = std::function<std::string(std::string, size_t)>;

bool MapReduce(const std::string& file_name, const std::vector<std::pair<size_t, size_t>> ranges, 
	MapReduceFunc func, size_t prefix_count);

