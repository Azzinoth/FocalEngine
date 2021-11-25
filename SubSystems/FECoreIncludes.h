#pragma once
#pragma warning (disable: 4752)     // found Intel(R) Advanced Vector Extensions; consider using / arch:AVX	FocalEnginePrivate

#include "FETime.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "GL/glew.h"
#include "GL/wglew.h"

#include <GLFW/glfw3.h>
#include <GL/GL.h>

#include <iostream>
#include <fstream>
#include <sstream>

// run time checking(/RTC) makes the Microsoft implementation of std containers 
// *very* slow in debug builds !
// So in visual studio go to Project -> Properties -> C/C++ -> Code Generation -> Basic Runtime Checks and set to "Default"
#include <unordered_map>
#include <map>
#define GLM_FORCE_XYZW_ONLY
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/quaternion.hpp"

#include "jsoncpp/json/json.h"

#include <xmmintrin.h>
#include <stdlib.h>

#include <thread>
#include <atomic>
#include <functional>
#include <random>

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f

#define SINGLETON_PUBLIC_PART(CLASS_NAME)  \
static CLASS_NAME& getInstance()           \
{										   \
	if (!_instance)                        \
		_instance = new CLASS_NAME();      \
	return *_instance;				       \
}                                          \
										   \
~CLASS_NAME();

#define SINGLETON_PRIVATE_PART(CLASS_NAME) \
static CLASS_NAME* _instance;              \
CLASS_NAME();                              \
CLASS_NAME(const CLASS_NAME &);            \
void operator= (const CLASS_NAME &);

#ifdef FE_DEBUG_ENABLED
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
		GLenum error = glGetError();	   \
		if (error != 0)                    \
		{								   \
			assert("FE_GL_ERROR" && 0);	   \
		}                                  \
	}
#else
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
	}
#endif // FE_GL_ERROR

#define FE_MAP_TO_STR_VECTOR(map)          \
std::vector<std::string> result;           \
auto iterator = map.begin();               \
while (iterator != map.end())              \
{                                          \
	result.push_back(iterator->first);     \
	iterator++;                            \
}                                          \
                                           \
return result;

static double FENextPowerOfTwo(double currentNumber)
{
	double next = 0.0;
	int iteration = 0;
	while (currentNumber >= next)
	{
		next = pow(2.0, iteration);
		iteration++;
	}                         

	return next;
}

#define FE_MAX_LIGHTS 10
#define FE_CSM_UNIT 28
#define FE_WIN_32
#define FE_MESH_VERSION 0.01f
#define FE_TEXTURE_VERSION 0.01f

#define FE_SIMD_ENABLED

//#define FE_OLD_LOAD
//#define FE_OLD_SAVE

//#define FE_GPUMEM_ALLOCATION_LOGING

#define USE_DEFERRED_RENDERER
//#define USE_SSAO
#define USE_GPU_CULLING

#ifdef USE_DEFERRED_RENDERER
	#define FE_CLEAR_COLOR glm::vec4(pow(0.55f, -2.2f), pow(0.73f, -2.2f), pow(0.87f, -2.2f), 1.0f)
#else
	#define FE_CLEAR_COLOR glm::vec4(0.55f, 0.73f, 0.87f, 1.0f)
#endif // USE_DEFERRED_RENDERER

static std::string getUniqueId()
{
	static std::random_device randomDevice;
	static std::mt19937 mt(randomDevice());
	static std::uniform_int_distribution<int> distribution(0, 128);

	static bool firstInitialization = true;
	if (firstInitialization)
	{
		srand(unsigned int(time(NULL)));
		firstInitialization = false;
	}

	std::string ID = "";
	ID += char(distribution(mt));
	for (size_t j = 0; j < 11; j++)
	{
		ID.insert(rand() % ID.size(), 1, char(distribution(mt)));
	}

	return ID;
}

// This function can produce ID's that are identical but it is extremely rare
// to be 100% sure I could implement system to prevent it but for the sake of simplicity I choose not to do that, at least for now.
// ID is a 24 long string.
static std::string getUniqueHexID()
{
	std::string ID = getUniqueId();
	std::string IDinHex = "";

	for (size_t i = 0; i < ID.size(); i++)
	{
		IDinHex.push_back("0123456789ABCDEF"[(ID[i] >> 4) & 15]);
		IDinHex.push_back("0123456789ABCDEF"[ID[i] & 15]);
	}

	std::string additionalRandomness = getUniqueId();
	std::string additionalString = "";
	for (size_t i = 0; i < ID.size(); i++)
	{
		additionalString.push_back("0123456789ABCDEF"[(additionalRandomness[i] >> 4) & 15]);
		additionalString.push_back("0123456789ABCDEF"[additionalRandomness[i] & 15]);
	}
	std::string finalID = "";

	for (size_t i = 0; i < ID.size() * 2; i++)
	{
		if (rand() % 2 - 1)
		{
			finalID += IDinHex[i];
		}
		else
		{
			finalID += additionalString[i];
		}
	}

	return finalID;
}

static bool setClipboardText(std::string text)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
		memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
		GlobalUnlock(hMem);

		SetClipboardData(CF_TEXT, hMem);

		CloseClipboard();
		return true;
	}

	return false;
}

static std::string getClipboardText()
{
	std::string text = "";

	if (OpenClipboard(0))
	{
		HANDLE data = nullptr;
		data = GetClipboardData(CF_TEXT);
		if (data != nullptr)
		{
			char* pszText = static_cast<char*>(GlobalLock(data));
			if (pszText != nullptr)
				text = pszText;
		}

		CloseClipboard();
	}

	return text;
}