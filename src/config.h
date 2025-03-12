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
	int shapeType;
	int startP;
};

const Shape shapes[] =
{
	// ID, Offset 
	{3, 0}
};
#define SHAPES_COUNT 1

#define endregion


// Count of mandatory data per vertex 
#define ATTRIBUTE_COUNT 10