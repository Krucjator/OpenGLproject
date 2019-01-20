#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MyLookAt.h"

class StaticCamera : public Camera
{
public:
	// Camera Attributes
	glm::vec3 TargetPosition;
	glm::vec3 Up;
	//glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
	

	StaticCamera(glm::vec3 pos, glm::vec3 targetPos, glm::vec3 up) {
		Position = pos;
		TargetPosition = targetPos;
		Up = up;
	}

	glm::mat4 GetViewMatrix()
	{
		return myLookAt(Position, TargetPosition, Up);
	}
};