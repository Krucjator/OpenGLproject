#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MyLookAt.h"

class FollowCamera : public Camera
{
public:
	// Camera Attributes
	//glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);


	FollowCamera(glm::vec3 pos) {
		Position = pos;
	}

	glm::mat4 GetViewMatrix(glm::vec3 targetPos, glm::vec3 carUp = glm::vec3(0.0001f,1.0f,0.0f))
	{
		return myLookAt(Position, targetPos, carUp);
	}
};