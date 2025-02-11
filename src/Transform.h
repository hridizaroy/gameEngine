#pragma once

#include "config.h"
#include "cmath"

// Modified from: https://github.com/vixorien/AdvancedDX11Starter/blob/main/Transform.h

class Transform
{
public:
	Transform();

	// Transformers
	void MoveAbs(float x, float y, float z);
	void MoveAbs(glm::vec3 offset);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(glm::vec3 offset);
	void RotateEuler(float p, float y, float r);
	void RotateEuler(glm::vec3 pitchYawRoll);
	void Scale(float uniformScale);
	void Scale(float x, float y, float z);
	void Scale(glm::vec3 scale);

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(glm::vec3 position);
	void SetEulerRotation(float p, float y, float r);
	void SetEulerRotation(glm::vec3 pitchYawRoll);
	void SetScale(float uniformScale);
	void SetScale(float x, float y, float z);
	void SetScale(glm::vec3 scale);

	// Getters
	glm::vec3 GetPosition();
	glm::vec3 GetEulerRotation();
	glm::vec3 GetScale();

	// Local direction vector getters
	glm::vec3 GetUp();
	glm::vec3 GetRight();
	glm::vec3 GetForward();

	// Matrix getters
	glm::mat4x4 GetWorldMatrix();
	glm::mat4x4 GetWorldInverseTransposeMatrix();
	
private:
	// Raw transformation data
	glm::vec3 position;
	glm::vec4 quatRot;
	glm::vec3 scale;

	// Local orientation vectors
	bool dirIsDirty;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 forward;

	// World matrix and inverse transpose of the world matrix
	bool matIsDirty;
	glm::mat4x4 worldMatrix;
	glm::mat4x4 worldInverseTransposeMatrix;

	// Helper to update both matrices if necessary
	void CleanMatrices();
	void CleanVectors();
};

inline static glm::vec3 QuatRot(glm::vec4 q, glm::vec3 v)
{
	// Reference: https://stackoverflow.com/questions/44705398/about-glm-quaternion-rotation
	glm::vec3 c = glm::cross(
		glm::vec3(q.x, q.y, q.z),
		cross(glm::vec3(q.x, q.y, q.z), v) + q.w * v);
	return v + 2.0f * c;
}

inline static glm::vec4 ToQuat(double roll, double pitch, double yaw) // roll (x), pitch (y), yaw (z), angles are in radians
{
	// Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

	// Abbreviations for the various angular functions

	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);

	glm::vec4 q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

static inline glm::vec3 ToEuler(glm::vec4 q) 
{

	// Reference: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

	glm::vec3 angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.z = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
	double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
	angles.x = 2 * std::atan2(sinp, cosp) - (2 * asin(1.0f)) / 2;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.y = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}