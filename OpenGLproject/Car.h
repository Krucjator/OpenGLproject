#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class Car {
public:
	glm::vec3 Position;
	float Yaw;
	float RotationSpeed;
	
	Car(glm::vec3 startingPos,float yaw, float rotationSpeed) {
		Position = startingPos;
		Yaw = yaw;
		RotationSpeed = rotationSpeed;
	}

	Car() {

	}
};