#include "Utils.h"

#include <windows.h>
#include <iosfwd>
#include <fstream>

namespace Utils
{
	std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
	
	uint64_t GetMS()
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		LONG time_ms = (time.wSecond * 1000) + time.wMilliseconds;
		return time_ms;
	}
};