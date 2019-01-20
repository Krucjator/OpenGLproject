#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


glm::mat4 myLookAt(glm::vec3 camPos, glm::vec3 targetPos, glm::vec3 up)
{
	glm::vec3 cameraDirection = glm::normalize(camPos - targetPos);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	glm::mat4 cameraSpace = glm::mat4(glm::vec4(cameraRight, 0.0f), glm::vec4(cameraUp, 0.0f), glm::vec4(cameraDirection, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glm::mat4 cameraPosition = glm::mat4(1.0f);
	cameraPosition[0].w = -camPos.x;
	cameraPosition[1].w = -camPos.y;
	cameraPosition[2].w = -camPos.z;
	glm::mat4 lookAt = cameraPosition * cameraSpace;
	//column-major order is default for glm and OpenGL, above i used row-major
	lookAt = glm::transpose(lookAt);

	return lookAt;
}