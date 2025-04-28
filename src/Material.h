#pragma once

#include <vector>

struct Material
{
	std::string vs;
	std::string ps;
	std::vector<float> floatData; // Todo: Don't do this 
};