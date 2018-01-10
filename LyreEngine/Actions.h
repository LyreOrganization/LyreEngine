#pragma once

enum class Action {
	DummyAction = 0,

	Exit,

	// Camera
	Camera_Tilt,
	Camera_Pan,
	Camera_MoveForward,
	Camera_MoveBackward,
	Camera_MoveRight,
	Camera_MoveLeft,
};

std::istream& operator>>(std::istream& is, Action& action);
