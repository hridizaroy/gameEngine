#include "EditorGUI.h"
#include "config.h"

#define _USE_MATH_DEFINES
#include <math.h>

EditorGUI::EditorGUI()
{
	activeType = EMPTY;


	UInfo i = { "Sample", std::make_shared<Transform>() };
	std::shared_ptr<UInfo> info = std::make_shared<UInfo>(i);

	rasterEntity = new REntity();
	rasterEntity->info = info;

	Shape shape = { glm::vec4(SPHERE, 0, 0, 0) };
	shapeEntity = new SEntity();
	shapeEntity->info = info;
	shapeEntity->shape = shape;
	shapeEntity->parameteres = new glm::vec4[ShapeTypes::GetShapeParamSize(SPHERE)];
	memset(shapeEntity->parameteres, 0, ShapeTypes::GetShapeParamSize(SPHERE) * sizeof(glm::vec4));
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
/// Generates GUI to allow a user to add an entity 
/// </summary>
/// <returns></returns>
GenerationRequest EditorGUI::AddEntityGUI()
{
	// Draw selection GUI 

	ImGui::Dummy(ImVec2(0.0f, 20.0f));

	
	
	// Combo for selecting gui type 
	const char* items[] = { "RASTER_ENTITY", "SHAPE_ENTITY"};
	static const char* current_item = items[0];
	static int current = 0;

	if (ImGui::BeginCombo(" ", current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (current_item != items[n]); // New item 
			if (ImGui::Selectable(items[n], is_selected))
			{
				current_item = items[n];
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					current = n;

					// Entity has been selected 

				}
			}
		}

		// Update details to creation based on selection 

		ImGui::EndCombo();
	}

	ImGui::SameLine();
	bool v = ImGui::Button("Add Entity");




	// Shape Type combo 


	



	// Draw editors based on raster or shape 
	switch (current)
	{
	case 0: // Raster
		CreateREntityInspectGUI(rasterEntity);
		break;
	case 1: // Shape 
		//CreateSEntityInspectGUI(shapeEntity);
		CreateSEntityMaker(shapeEntity);
		break; 
	}


	GenerationRequest request;

	// Has the button been pressed? 
	if (v)
	{
		// Decide what type of entity to return 
		switch (current)
		{
		case 0: // Raster
			request.type = RASTER_ENTITY;
			request.data = rasterEntity;
			return request;
		case 1: // Shape 
			request.type = SHAPE_ENTITY;
			request.data = shapeEntity;
			return request;
		}
	}

	// Default return null 
	request.type = EMPTY;
	return request;
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

/// <summary>
/// Generates a gui for a single SEntity intended for the inspector 
/// </summary>
/// <param name="entity"></param>
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




	static std::vector<glm::vec4> parameters = std::vector<glm::vec4>(10);

	// TODO: Check if amount of parameters needs to be increased dynamically 

	// TODO: Reset parameters on shape type change 

	const int shapeComboID = (int)"SHAPES";
	static const char* shapes[4] = 
	{
		"SPHERE",
		"BOX",
		"ROUND_BOX",
		"FRAME_BOX"
	};
	static const char* currentShape = shapes[0];
	int itemCount = IM_ARRAYSIZE(shapes);

	static uint32_t currentCombo = 0; 
	if (CreateCombo(shapes, itemCount, currentShape, currentCombo, -1))
	{
		currentShape = shapes[currentCombo];
		//entity->shape.shapeInfo.x = currentComboValue;
	}
	//entity->SetShapeID(curr);




	uint32_t p = 0; // Current parameter in entity 
	glm::vec3 pos = glm::vec3
	{
		entity->parameteres[0].x,
		entity->parameteres[1].x,
		entity->parameteres[2].x
	};

	if (ImGui::DragFloat3("Position", &pos[0], 0.01))
	{
		//memcpy(&entity->parameteres[0], &pos[0], sizeof(float) * 3);

		entity->parameteres[p++].x = pos[0];
		entity->parameteres[p++].x = pos[1];
		entity->parameteres[p++].x = pos[2];
	}
	else
	{
		p += 3; 
	}

	switch (entity->GetShapeID())
	{
	case SPHERE:

		if (ImGui::DragFloat("Radius", &parameters[0][0], 0.01))
		{
			entity->parameteres[p++].x = parameters[0][0];
		}

		break;
	case BOX:

		if (ImGui::DragFloat3("Size", &parameters[0][0], 0.01))
		{
			entity->parameteres[p++].x = parameters[0][0];
			entity->parameteres[p++].x = parameters[1][0];
			entity->parameteres[p++].x = parameters[2][0];
		}

		break;
	case ROUND_BOX:
		break;
	case FRAME_BOX:

		if (ImGui::DragFloat3("Size", &parameters[0][0], 0.01))
		{
			entity->parameteres[p++].x = parameters[0][0];
			entity->parameteres[p++].x = parameters[1][0];
			entity->parameteres[p++].x = parameters[2][0];
		}

		break;

	default:
		break;
	}

	//ImGui::PopID();
}

void EditorGUI::CreateSEntityInspector(
	SEntity* entity, 
	static const char* shapes[],
	uint32_t itemCount, 
	static const char* currentShape)
{
	static uint32_t currentCombo = 0;
	if (CreateCombo(shapes, itemCount, currentShape, currentCombo, -1))
	{
		currentShape = shapes[currentCombo];
		//entity->shape.shapeInfo.x = currentComboValue;
	}
}

/// <summary>
/// Showcases the gui for the maker. Almost identical to 
/// the inspector version asides from a static value 
/// </summary>
/// <param name="entity"></param>
void EditorGUI::CreateSEntityMaker(SEntity* entity)
{

}

/// <summary>
/// Creates the gui for the inspector
/// </summary>
/// <param name="entity"></param>
void EditorGUI::CreateSEntityInspector(SEntity* entity)
{

}


/// <summary>
/// Change the current inspector data and layout
/// </summary>
void EditorGUI::UpdateInspector(void* data, InspectorType inspectorType)
{
	// Check if valid input 
	if (data == nullptr || inspectorType == EMPTY)
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


/// <summary>
/// Creates a combo used explicetly for the inspector. Only one at a time please 
/// </summary>
bool EditorGUI::CreateCombo(const char* items[], int itemCount, static const char* current_item, uint32_t& comboIndex, int id)
{
	ImGui::PushID(id);
	//const char* items[] = { "Sphere", "Box", "RoundBox", "FrameBox" };
	//static const char* current_item = items[0];

	//printf("%i \n", IM_ARRAYSIZE(items));

	bool newValue = false; 

	if (ImGui::BeginCombo(" ", current_item))
	{
		for (int n = 0; n < itemCount; n++)
		{
			bool is_selected = (current_item != items[n]); // New item 
			if (ImGui::Selectable(items[n], is_selected))
			{
				current_item = items[n];
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
					//currentComboValue = n;

					comboIndex = n;

					// New selection 
					newValue = true; 
				}
			}
		}

		// Update details to creation based on selection 

		ImGui::EndCombo();
	}
	ImGui::PopID();

	return newValue;

	//return currentComboValue;
}