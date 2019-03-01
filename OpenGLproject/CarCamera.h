#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MyLookAt.h"
#include <vector>
#include "Camera.h"

glm::mat4 myLookAt(glm::vec3 camPos, glm::vec3 targetPos, glm::vec3 up);

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ROTATIONSPEED = 40.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class CarCamera : public Camera
{
public:
	// Camera Attributes
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	//Car
	glm::vec3 CarPosition;
	float CarYaw;
	glm::vec3 CarFront;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	// CarBelowCamera
	glm::vec3 Shift = glm::vec3(0.0f, -1.0f, 0.0f);

	// Constructor with vectors
	CarCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
	{
		Position = position;
		CarPosition = position;
		WorldUp = up;
		Yaw = yaw;
		CarYaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		updateCarFront();
	}
	// Constructor with scalar values
	//CarCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
	//{
	//	Position = glm::vec3(posX, posY, posZ);
	//	WorldUp = glm::vec3(upX, upY, upZ);
	//	Yaw = yaw;
	//	Pitch = pitch;
	//	car = Car(glm::vec3(posX, -1.0f, posZ), YAW, ROTATIONSPEED);
	//	updateCameraVectors();
	//}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return myLookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		float rvelocity = ROTATIONSPEED * deltaTime;
		if (direction == FORWARD) {
			Position += CarFront * velocity;
		}	
		if (direction == BACKWARD) {
			Position -= CarFront * velocity;
			
		}
		if (direction == LEFT) {
			CarYaw -= rvelocity;
			updateCarFront();
			//updateCameraVectors();
		}
		if (direction == RIGHT) {
			CarYaw += rvelocity;
			updateCarFront();
			//updateCameraVectors();
		}
		
	/*	if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;*/
	}

	glm::vec3 GetCarPosition() {
		return Position + Shift;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void updateCarFront() {
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(CarYaw)) * cos(glm::radians(0.0f));
		front.y = sin(glm::radians(0.0f));
		front.z = sin(glm::radians(CarYaw)) * cos(glm::radians(0.0f));
		CarFront = glm::normalize(front);
	}
};

//glm::mat4 myLookAt(glm::vec3 camPos, glm::vec3 targetPos, glm::vec3 up)
//{
//	glm::vec3 cameraDirection = glm::normalize(camPos - targetPos);
//	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
//	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
//	glm::mat4 cameraSpace = glm::mat4(glm::vec4(cameraRight, 0.0f), glm::vec4(cameraUp, 0.0f), glm::vec4(cameraDirection, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
//	glm::mat4 cameraPosition = glm::mat4(1.0f);
//	cameraPosition[0].w = -camPos.x;
//	cameraPosition[1].w = -camPos.y;
//	cameraPosition[2].w = -camPos.z;
//	glm::mat4 lookAt = cameraPosition * cameraSpace;
//	//column-major order is default for glm and OpenGL, above i used row-major
//	lookAt = glm::transpose(lookAt);
//	return lookAt;
//}

#endif