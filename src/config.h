#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum MeshType
{
	TRIANGLE,
	PENTAGON,
	HEXAGON,
	TRIANGLE_FULLSCREEN
};

// Count of mandatory data per vertex 
#define ATTRIBUTE_COUNT 10