#include "Leia_3D_Manager.h"

#include "sr/management/srcontext.h"
#include "sr/sense/display/switchablehint.h"
#include "sr/utility/exception.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetLeia3DManager()
{
	return Leia3DManager::GetInstancePointer();
}
#endif

Leia3DManager::Leia3DManager()
{
}

Leia3DManager::~Leia3DManager()
{
	if (Context != nullptr)
		SR::SRContext::deleteSRContext(Context);
}

bool Leia3DManager::InitializeContext(const bool bInitialLensState)
{
	if (Context != nullptr)
		return true;

	try
	{
		Context = SR::SRContext::create(bInitialLensState);
	}
	catch (const SR::ServerNotAvailableException&)
	{
		LOG.Add("In function Leia3DManager::InitializeContext could not connect to the LeiaSR service, it might not be running.", "FE_LOG_LEIA_3D", FE_LOG_ERROR);
		Context = nullptr;
		return false;
	}

	LensHint = SR::SwitchableLensHint::create(*Context);
	Context->initialize();

	return true;
}

bool Leia3DManager::Set3DModeEnabled(const bool bNewValue)
{
	if (Context == nullptr)
		return false;

	bNewValue ? LensHint->enable() : LensHint->disable();

	b3DModeEnabled = bNewValue;
	return true;
}

bool Leia3DManager::Is3DModeEnabled() const
{
	return b3DModeEnabled;
}