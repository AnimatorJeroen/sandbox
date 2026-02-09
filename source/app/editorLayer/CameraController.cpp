#include "pch.h"
#include "CameraController.h"
#include "app/sceneLayer/components/Components.h"
#include <glm/gtc/matrix_transform.hpp>

CameraController::CameraController()
{
}

void CameraController::OnMouseMove(double mouseX, double mouseY, 
	bool isLeftButtonDown, bool isMiddleButtonDown, bool isRightButtonDown)
{
	if (_firstMove)
	{
		_lastMouseX = mouseX;
		_lastMouseY = mouseY;
		_firstMove = false;
		return;
	}

	float deltaX = static_cast<float>(mouseX - _lastMouseX);
	float deltaY = static_cast<float>(mouseY - _lastMouseY);

	_lastMouseX = mouseX;
	_lastMouseY = mouseY;

	// Left button: Orbit
	if (isRightButtonDown && !isMiddleButtonDown)
	{
		OrbitCamera(deltaX, -deltaY);
	}
	// Middle button or Alt+Left: Pan
	else if (isLeftButtonDown || isMiddleButtonDown || (isRightButtonDown && isLeftButtonDown))
	{
		PanCamera(deltaX, deltaY);
	}
}

void CameraController::OnMouseScroll(double scrollY)
{
	ZoomCamera(static_cast<float>(scrollY));
}

void CameraController::OnMouseDown(double mouseX, double mouseY)
{
	_lastMouseX = mouseX;
	_lastMouseY = mouseY;
	_firstMove = false;
}

void CameraController::OrbitCamera(float deltaX, float deltaY)
{
	if(!_camera)
		return;
	// Calculate orbit angles
	float azimuthDelta = -deltaX * _orbitSensitivity;
	float elevationDelta = -deltaY * _orbitSensitivity;

	// Calculate current camera direction and distance
	glm::vec3 direction = _camera->position - _camera->target;
	float distance = glm::length(direction);
	
	if (distance < 0.01f)
		return;

	// Convert to spherical coordinates
	float currentAzimuth = atan2(direction.x, direction.z);
	float currentElevation = asin(direction.y / distance);

	// Update angles
	float newAzimuth = currentAzimuth + azimuthDelta;
	float newElevation = currentElevation + elevationDelta;

	// Clamp elevation to avoid gimbal lock
	const float maxElevation = glm::radians(89.0f);
	newElevation = glm::clamp(newElevation, -maxElevation, maxElevation);

	// Convert back to Cartesian coordinates
	float cosElevation = cos(newElevation);
	glm::vec3 newDirection;
	newDirection.x = distance * cosElevation * sin(newAzimuth);
	newDirection.y = distance * sin(newElevation);
	newDirection.z = distance * cosElevation * cos(newAzimuth);

	// Update camera position (orbit around target)
	_camera->position = _camera->target + newDirection;
}

void CameraController::PanCamera(float deltaX, float deltaY)
{
	if (!_camera)
		return;

	// Calculate camera right and up vectors
	glm::vec3 forward = glm::normalize(_camera->target - _camera->position);
	glm::vec3 right = glm::normalize(glm::cross(forward, _camera->up));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	// Calculate pan offset based on camera distance
	glm::vec3 direction = _camera->position - _camera->target;
	float distance = glm::length(direction);
	float panScale = distance * _panSensitivity;

	// Pan both camera and target
	glm::vec3 panOffset = right * (-deltaX * panScale) + up * (deltaY * panScale);
	_camera->position += panOffset;
	_camera->target += panOffset;
}

void CameraController::ZoomCamera(float zoomDelta)
{
	if (!_camera)
		return;
	// Calculate zoom direction
	glm::vec3 direction = _camera->position - _camera->target;
	float distance = glm::length(direction);
	
	if (distance < 0.01f)
		return;

	// Calculate zoom amount (percentage of current distance)
	float zoomAmount = -zoomDelta * _zoomSensitivity * distance;
	
	// Calculate new distance
	float newDistance = distance + zoomAmount;
	
	// Clamp minimum distance to avoid getting too close
	const float minDistance = 0.1f;
	const float maxDistance = 100000.0f;
	newDistance = glm::clamp(newDistance, minDistance, maxDistance);

	// Update camera position
	glm::vec3 normalizedDirection = direction / distance;
	_camera->position = _camera->target + normalizedDirection * newDistance;
}
