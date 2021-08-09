#include "map_reduce.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <future>

using MrVector = std::shared_ptr<std::vector<std::string>>;

void MapThread(const std::string& file_name, size_t begin, size_t end, MapFunc& func, size_t prefix_count, MrVector& output)
{
	std::ifstream ifs(file_name);

	ifs.seekg(begin);

	std::string line;

	while (ifs.tellg() < std::streamoff(end))
	{
		ifs >> line;
		output->push_back(func(line, prefix_count));
		if (ifs.tellg() == -1) break;
	}

	std::sort(output->begin(), output->end());
}

bool ReduceThread(MrVector& input, ReduceFunc& func)
{
	return func(*input);
}

bool AllMapBuffersIsEmpty( const std::vector<MrVector>& map_output )
{
	for( const auto& ptr : map_output )
	{
		if(!ptr->empty()) return false;
	}

	return true;
}

void CopyAllDuplicates(const std::string duplicate, MrVector reduce_input, std::vector<MrVector>& map_output)
{
	for( auto const& ptr : map_output )
	{
		while(true)
		{
			auto itr = std::find(ptr->begin(), ptr->end(), duplicate);

			if(itr == ptr->end()) break;

			ptr->erase(itr);
			reduce_input->push_back(duplicate);
		}
	}
}

void Shuffle(std::vector<MrVector>& map_output,	std::vector<MrVector>& reduce_input)
{
	for( size_t i = 0, j = 0;; ++i, ++j )
	{
		if(i == map_output.size()) i = 0;
		if(j == reduce_input.size()) j = 0;

		if(AllMapBuffersIsEmpty(map_output)) break;

		if(!map_output[i]->empty())
			CopyAllDuplicates(map_output[i]->front(), reduce_input[j], map_output );
	}
}

bool MapReduce(const std::string& file_name, const std::vector<std::pair<size_t, size_t>> ranges,
	size_t prefix_count, int rnum, MapFunc map_func, ReduceFunc reduce_func)
{
	std::vector<std::thread> threads;
	std::vector<std::shared_ptr<std::vector<std::string>>> map_output;
	map_output.reserve(ranges.size());
	std::vector<std::shared_ptr<std::vector<std::string>>> reduce_input;
	reduce_input.reserve(rnum);

	// Map

	for (const auto& pair : ranges)
	{
		map_output.emplace_back(std::make_shared<std::vector<std::string>>());
		threads.push_back(std::thread(MapThread, ref(file_name), pair.first, pair.second, std::ref(map_func),
			prefix_count, std::ref(map_output.back())));
	}

	for (auto& thr : threads)
		thr.join();

	// Shuffle

	for(int i = 0; i < rnum; ++i)
		reduce_input.emplace_back(std::make_shared<std::vector<std::string>>());

	Shuffle(map_output, reduce_input);

	// Reduce

	std::vector<std::future<bool>> futures;

	for (auto& ptr : reduce_input)
	{
		auto future = std::async(ReduceThread, std::ref(ptr), std::ref(reduce_func));
		futures.push_back(std::move(future));
	}

	for (auto& future : futures)
	{
		if (future.get() == false)
			return false;
	}
	
	return true;
}
