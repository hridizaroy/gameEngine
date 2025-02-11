#pragma once

#include "config.h"

#include "Transform.h"
#include "Mesh.h"

/// <summary>
/// Information used by both raster and shape entities 
/// </summary>
struct UInfo
{
	std::string name; // Hierachy displayed name 
	std::shared_ptr<Transform> transform;
};

struct SEntity
{
	std::shared_ptr<UInfo> info;
	uint32_t shapeType; 
};

struct REntity
{
	std::shared_ptr<UInfo> info; 
	//std::shared_ptr<Mesh> mesh;
	MeshType meshType; // Todo: Remove to be generic mesh 
};