#include "EditorGUI.h"
#include "config.h"

#define _USE_MATH_DEFINES
#include <math.h>

EditorGUI::EditorGUI()
{
	activeType = EMPTY;
}

/// <summary>
/// Generates a button to select a raster entity 
/// </summary>
bool EditorGUI::CreateREntitySelectGUI(REntity* entity, uint32_t id)
{
	ImGui::PushID(id);
	bool v = ImGui::Button(entity->info->name.c_str());
	ImGui::PopID();
	return v;
}

/// <summary>
/// Generates a button to select a shape entity 
/// </summary>
bool EditorGUI::CreateSEntitySelectGUI(SEntity* entity, uint32_t id)
{
	ImGui::PushID(id);
	bool v = ImGui::Button(entity->info->name.c_str());
	ImGui::PopID();
	return v;
}

/// <summary>
/// Generates a gui for a single REntity intended for the inspector 
/// </summary>
void EditorGUI::CreateREntityInspectGUI(REntity* entity)
{
	std::shared_ptr<Transform> trans = entity->info->transform;

	glm::vec3 pos = trans->GetPosition();
	glm::vec3 rot = trans->GetEulerRotation();
	glm::vec3 sca = trans->GetScale();

	glm::vec3 holdRot = rot; 

	//ImGui::PushID(id);
	ImGui::Text(entity->info->name.c_str());

	if (ImGui::DragFloat3("Position", &pos[0], 0.01f))
	{
		trans->SetPosition(pos);
	}
	if (ImGui::DragFloat3("Rotation", &rot[0], 0.01f))
	{
		glm::quat rotQuat = glm::quat(rot);
		trans->SetRotation(rotQuat);
	}
	if (ImGui::DragFloat3("Scale", &sca[0], 0.01f))
	{
		trans->SetScale(sca);
	}

	//ImGui::PopID();
}

void EditorGUI::CreateSEntityInspectGUI(SEntity* entity)
{
	// TODO: When reading from file that holds shape data
	//		 make it use that data to generate ui 


	// NOTE: Current UI is written by hand and needs to be
	//		 updated to be able to take in arbitrary params 

	// Draws a different UI based on the shape 

	uint32_t shapeType = entity->GetShapeID();

	//ImGui::PushID(id);
	ImGui::Text(entity->info->name.c_str());

	glm::vec3 pos = glm::vec3
	{ 
		entity->parameteres[0].x, 
		entity->parameteres[1].x,
		entity->parameteres[2].x
	};

	if (ImGui::DragFloat3("Position", &pos[0], 0.01))
	{
		//memcpy(&entity->parameteres[0], &pos[0], sizeof(float) * 3);

		entity->parameteres[0].x = pos[0];
		entity->parameteres[1].x = pos[1];
		entity->parameteres[2].x = pos[2];
	}

	//ImGui::PopID();
}



/// <summary>
/// Change the current inspector data and layout
/// </summary>
void EditorGUI::UpdateInspector(void* data, InspectorType inspectorType)
{
	// Check if valid input 
	if (data == nullptr && inspectorType != EMPTY)
	{
		return;
	}

	// Update data 
	activeData = data;
	activeType = inspectorType;
}

/// <summary>
/// Generates inspector information specific 
/// </summary>
void EditorGUI::DrawInspector()
{
	
	switch (activeType)
	{
	case EMPTY:
		break;
	case RASTER_ENTITY:
		CreateREntityInspectGUI((REntity*)activeData);
		break;
	case SHAPE_ENTITY:
		CreateSEntityInspectGUI((SEntity*)activeData);
		break;
	case CAMERA:
		break;
	default:
		break;
	}

}