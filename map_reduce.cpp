#include "map_reduce.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

void MapThread(const std::string& file_name, size_t begin, size_t end, std::mutex& mt, MapReduceFunc& func,
	size_t prefix_count, std::shared_ptr<std::vector<std::string>>& output)
{
	std::lock_guard<std::mutex> lock(mt); // !!!!!!!!!!!!!!!!

	//std::cout << "begin: " << begin << " end: " << end << std::endl;

	std::ifstream ifs(file_name);

	ifs.seekg(begin);

	size_t size = 0;

	std::string line;

	while (ifs && ifs.tellg() < end)
	{
		ifs >> line;
		//std::cout << line << std::endl;
		output->push_back(func(line, prefix_count));
		size += line.size();
	}
}

bool MapReduce(const std::string& file_name, const std::vector<std::pair<size_t, size_t>> ranges, MapReduceFunc func, size_t prefix_count)
{
	std::vector<std::thread> threads;
	std::mutex mutex;
	std::vector<std::shared_ptr<std::vector<std::string>>> map_output;
	map_output.reserve(ranges.size());

	for (const auto& pair : ranges)
	{
		auto output = std::make_shared<std::vector<std::string>>();
		map_output.emplace_back(output);
		threads.push_back(std::thread(MapThread, ref(file_name), pair.first, pair.second, std::ref(mutex), std::ref(func), prefix_count, std::ref(output)));
	}

	for (auto& thr : threads)
		thr.join();
	
	for (const auto& ptr : map_output)
	{
		std::cout << "op\n";
		for (const auto& s : *ptr)
			std::cout << s << std::endl;
	}
	
	return false;
}
