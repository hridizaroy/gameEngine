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
	void CreateREntityInspectGUI(REntity* entity, uint32_t id);

	void UpdateInspector(REntity* entity);
	void DrawInspector();

private:
	REntity* activeEntity;
};