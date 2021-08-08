#include "map.hpp"

#include <iostream>
#include <fstream>


void MapThread(const std::string& file_name, size_t begin, size_t end, std::mutex& mt)
{
	std::lock_guard<std::mutex> lock(mt);

	//std::cout << "begin: " << begin << " end: " << end << std::endl;

	std::ifstream ifs(file_name);

	ifs.seekg( begin );

	size_t size = 0;

	std::string line;

	while (ifs && ifs.tellg() < end)
	{
		ifs >> line;
		std::cout << line << std::endl;
		size += line.size();
	}

}