#include <mutex>

void MapThread(const std::string& file_name, size_t begin, size_t end, std::mutex& mt);