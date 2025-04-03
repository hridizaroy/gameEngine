#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include "Entity.h"

enum InspectorType
{
	EMPTY, 
	RASTER_ENTITY,
	SHAPE_ENTITY,
	CAMERA
};

struct GenerationRequest
{
	InspectorType type;
	void* data; 
};

class EditorGUI
{
public:
	EditorGUI();

	// Hierach side 
	// NOTE: Returns information useful for the engine  

	bool CreateREntitySelectGUI(REntity* entity, uint32_t id);
	bool CreateSEntitySelectGUI(SEntity* entity, uint32_t id);
	
	GenerationRequest AddEntityGUI();


	// Inspector side 

	void CreateREntityInspectGUI(REntity* entity);
	void CreateSEntityInspectGUI(SEntity* entity);
	void UpdateInspector(void* data, InspectorType inspectorType);
	void DrawInspector();

private:

	// TODO: Change to either enum or arbitrary inspector

	//bool showRaster; // Whether to show raster or shape entity 
	//REntity* activeREntity;
	//SEntity* activeSEntity;

	void*		  activeData; 
	InspectorType activeType;

	// Used for representing entity info on the hierachy side 

	REntity* rasterEntity;
	SEntity* shapeEntity;
};