#include "map_reduce.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

void MapThread(const std::string& file_name, size_t begin, size_t end, std::mutex& /*mt*/, MapFunc& func,
	size_t prefix_count, std::shared_ptr<std::vector<std::string>>& output)
{
	//std::lock_guard<std::mutex> lock(mt); // !!!!!!!!!!!!!!!!
	std::cout << "thread begin\n";
	std::cout << "begin: " << begin << " end: " << end << std::endl;

	std::ifstream ifs(file_name);

	ifs.seekg(begin);

	size_t size = 0;

	std::string line;

	while (ifs && ifs.tellg() < std::streamoff(end))
	{
		ifs >> line;
		//std::cout << line << std::endl;
		output->push_back(func(line, prefix_count));
		size += line.size();
	}

	std::sort(output->begin(), output->end());

	std::cout << "thread end\n";
}

void ReduceThread(std::shared_ptr<std::vector<std::string>>& input, ReduceFunc& func)
{
	std::cout << "Reduce result " << func(*input) << std::endl;
}

bool AllMapBuffersIsEmpty( const std::vector<std::shared_ptr<std::vector<std::string>>>& map_output )
{
	for( const auto& ptr : map_output )
	{
		if(!ptr->empty()) return false;
	}

	return true;
}

void CopyAllDuplicates(const std::string duplicate, std::shared_ptr<std::vector<std::string>> reduce_input,
	std::vector<std::shared_ptr<std::vector<std::string>>>& map_output)
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

void Shuffle(std::vector<std::shared_ptr<std::vector<std::string>>>& map_output,
	std::vector<std::shared_ptr<std::vector<std::string>>>& reduce_input)
{
	for( size_t i = 0, j = 0;; ++i, ++j )
	{
		if(i == map_output.size()) i = 0;
		if(j == reduce_input.size()) j = 0;

		if(AllMapBuffersIsEmpty(map_output)) break;

		CopyAllDuplicates(map_output[i]->front(), reduce_input[j], map_output );
/*
		std::cout << "REDUCE\n";

		for (const auto& ptr : reduce_input)
		{
			std::cout << "_____________\n";
			for (const auto& s : *ptr)
				std::cout << s << std::endl;
		}
*/		
	}
}

bool MapReduce(const std::string& file_name, const std::vector<std::pair<size_t, size_t>> ranges,
	size_t prefix_count, int rnum, MapFunc map_func, ReduceFunc reduce_func)
{
	std::vector<std::thread> threads;
	std::mutex mutex;
	std::vector<std::shared_ptr<std::vector<std::string>>> map_output;
	map_output.reserve(ranges.size());
	std::vector<std::shared_ptr<std::vector<std::string>>> reduce_input;
	reduce_input.reserve(rnum);

	for (const auto& pair : ranges)
	{
		map_output.emplace_back(std::make_shared<std::vector<std::string>>());
		threads.push_back(std::thread(MapThread, ref(file_name), pair.first, pair.second, std::ref(mutex), std::ref(map_func),
			prefix_count, std::ref(map_output.back())));
	}

	for (auto& thr : threads)
		thr.join();

	for (const auto& ptr : map_output)
	{
		std::cout << "_____________\n";
		for (const auto& s : *ptr)
			std::cout << s << std::endl;
	}

	for(int i = 0; i < rnum; ++i)
		reduce_input.emplace_back(std::make_shared<std::vector<std::string>>());

	Shuffle(map_output, reduce_input);

	std::cout << "REDUCE\n";

	for (auto& ptr : reduce_input)
	{
		std::cout << "_____________\n";
		for (const auto& s : *ptr)
			std::cout << s << std::endl;

		threads.push_back(std::thread(ReduceThread, std::ref(ptr), std::ref(reduce_func)));
	}

	for (auto& thr : threads)
		thr.join();
	
	return false;
}
