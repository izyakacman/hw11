#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>

#include "map.hpp"

using namespace std;

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

	ifstream ifs(file_name, std::ifstream::ate | std::ifstream::binary);
	size_t file_size = ifs.tellg();

	cout << "file size = " << file_size << endl;
	ifs.seekg(0);

	vector<thread> threads;
	size_t block_size = file_size / mnum;
	size_t begin = 0;
	mutex mt;

	for (int i = 0; i < mnum; ++i)
	{
		ifs.seekg(begin + block_size);
		string line;
		ifs >> line;
		size_t end = ifs.tellg();

		threads.push_back( thread(MapThread, ref(file_name), begin, end, ref(mt)) );
		begin = end+1;
	}

	for (auto& thr : threads)
		thr.join();

	return 0;
}
