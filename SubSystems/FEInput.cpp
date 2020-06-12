#include "FEInput.h"
using namespace FocalEngine;

FEInput* FEInput::_instance = nullptr;

FEInput::FEInput()
{

}

FEInput::~FEInput()
{

}

void FEInput::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	getInstance().mouseButtonCallbackImpl(button, action, mods);
}

void FEInput::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	getInstance().mouseMoveCallbackImpl(xpos, ypos);
}

void FEInput::keyButtonCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	getInstance().keyButtonCallbackImpl(key, scancode, action, mods);
}