#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <vector>

namespace Utils
{
	std::vector<char> readFile(const std::string& filename);
	uint64_t GetMS();
};

#endif // !__UTILS_H__
