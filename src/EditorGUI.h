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

	// Helpers 

	bool CreateCombo(const char* items[], int itemCount, static const char* current, uint32_t& comboIndex, int id);
	void CreateSEntityInspector(SEntity* entity);
	void CreateSEntityMaker(SEntity* entity);

	void CreateSEntityInspector(
		SEntity* entity,
		static const char* shapes[],
		uint32_t itemCount,
		static const char* currentShape);

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

	//uint32_t currentComboValue;
};