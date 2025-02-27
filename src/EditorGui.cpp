#include "EditorGUI.h"
#include "config.h"

EditorGUI::EditorGUI()
{

}

/// <summary>
/// Generates a gui for a single REntity 
/// </summary>
void EditorGUI::CreateREntityGUI(REntity& entity, uint32_t id)
{
	std::shared_ptr<Transform> trans = entity.info->transform;

	glm::vec3 pos = trans->GetPosition();
	glm::vec3 rot = trans->GetEulerRotation();
	glm::vec3 sca = trans->GetScale();


	ImGui::PushID(id);
	ImGui::Text(entity.info->name.c_str());

	if (ImGui::DragFloat3("Position", &pos[0], 0.01f))
	{
		trans->SetPosition(pos);
	}
	if (ImGui::DragFloat3("Rotation", &rot[0], 0.01f))
	{
		trans->SetEulerRotation(rot);
	}
	if (ImGui::DragFloat3("Scale", &sca[0], 0.01f))
	{
		trans->SetScale(sca);
	}

	ImGui::PopID();
}