#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include "Entity.h"

class EditorGUI
{
public:
	EditorGUI();

	bool CreateREntitySelectGUI(REntity* entity, uint32_t id);
	bool CreateSEntitySelectGUI(SEntity* entity, uint32_t id);
	void CreateREntityInspectGUI(REntity* entity, uint32_t id);
	void CreateSEntityInspectGUI(SEntity* entity, uint32_t id);

	void UpdateRasterInspector(REntity* entity);
	void UpdateShapeInspector(SEntity* entity);
	void DrawInspector();

private:

	// TODO: Change to either enum or arbitrary inspector

	bool showRaster; // Whether to show raster or shape entity 
	REntity* activeREntity;
	SEntity* activeSEntity;
};