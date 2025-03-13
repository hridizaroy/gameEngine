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


#define TEMPORARY

// Note: This section purely immitates what type of
//		 scene data we should expect as input 

struct Shape
{
	glm::uvec4 shapeInfo;

	//uint32_t shapeType;
	//uint32_t startP;
};

const Shape shapes[] =
{
	glm::uvec4(3, 0, 0, 0),
	glm::uvec4(0, 7, 0, 0)
	// ID, Offset 
	//{3, 0},
	//{0, 7}
};


#define SHAPES_COUNT 2

#define endregion


// Count of mandatory data per vertex 
#define ATTRIBUTE_COUNT 10