#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>

#include "map_reduce.hpp"

using namespace std;

constexpr int max_prefix_lenght = 100;

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		cout << "mapreduce <src> <mnum> <rnum>";
		return -1;
	}

	int mnum = stoi(argv[2]);
	int rnum = stoi(argv[3]);
	string file_name = argv[1];

	// Get ranges

	ifstream ifs(file_name, std::ifstream::ate | std::ifstream::binary);

	if (!ifs) return -1;

	size_t file_size = ifs.tellg();

	ifs.seekg(0);

	size_t block_size = file_size / mnum;
	size_t begin = 0;
	size_t end = 0;
	vector<pair<size_t, size_t>> ranges;

	for (int i = 0; i < mnum; ++i)
	{
		ifs.seekg(begin + block_size);
		string line;
		ifs >> line;
		end = ifs.tellg();
		
		if(end == static_cast<size_t>(-1))
			end = file_size - 1;

		ranges.push_back({begin, end});

		begin = end+1;
	}

	// Map-reduce

	int prefix_count = 1;

	for (; prefix_count < max_prefix_lenght; ++prefix_count)
	{
		bool res = MapReduce(file_name, ranges, prefix_count, rnum,
			[](string s, size_t count) // map
			{
				return s.substr(0, count);
			},
			[](std::vector<std::string> strings) // reduce`
			{
				std::string last_string;
				for (size_t i = 0; i < strings.size(); ++i)
				{
					if (i && strings[i] == last_string)
						return false;

					last_string = strings[i];
				}

				return true;
			});

		if (res) break;
	}

	if (prefix_count == max_prefix_lenght)
		std::cout << "Map-reduce error\n";
	else
		std::cout << "Minimal prefix lenght " << prefix_count << std::endl;

	return 0;
}
