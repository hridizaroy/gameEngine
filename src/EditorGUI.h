#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include "Entity.h"

class EditorGUI
{
public:
	EditorGUI();

	void CreateREntityGUI(REntity& entity, uint32_t id);

private:


};