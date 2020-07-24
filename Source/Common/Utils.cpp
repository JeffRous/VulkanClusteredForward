#include "Utils.h"

#include <windows.h>
#include <stdio.h>
#include <iosfwd>
#include <fstream>
#include <time.h>
#include <chrono>

using namespace std;

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
	
	double GetTimeEclapsed()
	{
		return (double)clock() / CLOCKS_PER_SEC;
	}

	static std::chrono::steady_clock::time_point start, end;
	void GetMSStart()
	{
		start = std::chrono::high_resolution_clock::now();
	}

	double GetMSEnd()
	{
		end = std::chrono::high_resolution_clock::now();
		return (double)chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
	}
};