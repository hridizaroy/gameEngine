#include "Transform.h"

Transform::Transform() :
	position(glm::vec3(0.0f, 0.0f, 0.0f)),
	quatRot(1.0f, 0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f)
{
	worldMatrix = glm::mat4x4();
	worldInverseTransposeMatrix = glm::mat4x4();

	matIsDirty = true;
	dirIsDirty = true;

	CleanVectors();
	CleanMatrices();
}

#pragma region HELPERS

void Transform::CleanMatrices()
{
	// Create a new world if transform has been mutated 
	if (matIsDirty)
	{
		// Get each of parts that represent the world matrix 
		glm::mat4x4 pos = glm::translate(glm::mat4(1.0f), position);
		glm::mat4x4 rot = glm::rotate(glm::mat4(1.0f), quatRot.w, glm::vec3(quatRot.x, quatRot.y, quatRot.z));
		glm::mat4x4 sca = glm::scale(glm::mat4(1.0f), scale);

		// Final results set here 
		worldMatrix = sca * rot * pos;
		worldInverseTransposeMatrix = glm::inverse(glm::transpose(worldMatrix));

		matIsDirty = false;
	}
}

void Transform::CleanVectors()
{
	if (!dirIsDirty)
		return;

	right = QuatRot(quatRot, glm::vec3(1.0, 0.0, 0.0));
	up = QuatRot(quatRot, glm::vec3(0.0, 1.0, 0.0));
	forward = QuatRot(quatRot, glm::vec3(0.0, 0.0, 1.0));
	
	dirIsDirty = false;
}
#pragma endregion 

#pragma region SETTERS

void Transform::SetPosition(float x, float y, float z)
{
	// Was there a XMFloat function for this? 
	position.x = x;
	position.y = y;
	position.z = z;

	matIsDirty = true;
}

void Transform::SetPosition(glm::vec3 position)
{
	position = position;
	matIsDirty = true;
}

void Transform::SetEulerRotation(float pitch, float yaw, float roll)
{
	quatRot = ToQuat(roll, pitch, yaw);

	matIsDirty = true;
	dirIsDirty = true;
}

void Transform::SetEulerRotation(glm::vec3 rotation)
{
	quatRot = ToQuat(rotation.x, rotation.y, rotation.z);

	matIsDirty = true;
	dirIsDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;

	matIsDirty = true;
}

void Transform::SetScale(glm::vec3 scale)
{
	this->scale = scale;

	matIsDirty = true;
}

void Transform::SetScale(float s)
{
	SetScale(s, s, s);

	matIsDirty = true;
}

#pragma endregion

#pragma region GETTERS
glm::vec3 Transform::GetPosition()
{
	return position;
}

glm::vec3 Transform::GetEulerRotation()
{
	return ToEuler(quatRot);
}

glm::vec3 Transform::GetScale()
{
	return scale;
}

glm::mat4x4 Transform::GetWorldMatrix()
{
	CleanMatrices();
	return worldMatrix;
}

glm::mat4x4 Transform::GetWorldInverseTransposeMatrix()
{
	CleanMatrices();
	return worldInverseTransposeMatrix;
}

glm::vec3 Transform::GetRight()
{
	if (dirIsDirty)
		CleanVectors();

	return right;
}

glm::vec3 Transform::GetUp()
{
	if (dirIsDirty)
		CleanVectors();

	return up;
}

glm::vec3 Transform::GetForward()
{
	if (dirIsDirty)
		CleanVectors();

	return forward;
}

#pragma endregion

#pragma region MUTATORS 
void Transform::MoveAbs(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	matIsDirty = true;
}

void Transform::MoveAbs(glm::vec3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
	matIsDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	// Creat the movement variable 
	glm::vec3 toMove = glm::vec3(x, y, z);

	// Convert to local space 
	toMove = QuatRot(quatRot, toMove);

	// Add in local space 
	toMove = position + toMove;

	// Store 
	position = toMove;
	matIsDirty = true;
}

void Transform::MoveRelative(glm::vec3 vec)
{
	glm::vec3 toMove = vec;

	// Convert to local space 
	toMove = QuatRot(quatRot, toMove);

	// Add in local space 
	toMove = position + toMove;

	// Store 
	position = toMove;

	matIsDirty = true;
}

void Transform::RotateEuler(float pitch, float yaw, float roll)
{
	glm::vec4 mutQuat = ToQuat(roll, pitch, yaw);
	quatRot = quatRot * mutQuat;

	matIsDirty = true;
	dirIsDirty = true;
}

void Transform::RotateEuler(glm::vec3 rotation)
{
	glm::vec4 mutQuat = ToQuat(rotation.z, rotation.x, rotation.y);
	quatRot = quatRot * mutQuat;

	matIsDirty = true;
	dirIsDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;

	matIsDirty = true;
}

void Transform::Scale(glm::vec3 scale)
{
	this->scale.x += scale.x;
	this->scale.y += scale.y;
	this->scale.z += scale.z;

	matIsDirty = true;
}

void Transform::Scale(float scale)
{
	this->scale.x += scale;
	this->scale.y += scale;
	this->scale.z += scale;

	matIsDirty = true;
}

#pragma endregion