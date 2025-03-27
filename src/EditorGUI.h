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

class EditorGUI
{
public:
	EditorGUI();

	bool CreateREntitySelectGUI(REntity* entity, uint32_t id);
	bool CreateSEntitySelectGUI(SEntity* entity, uint32_t id);
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
};