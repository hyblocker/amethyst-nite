#pragma once
#include <Windows.h>
#include <Ole2.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

#include <fstream>
#include <thread>

#define _MSC_VER 1500
#include "OpenNI.h"

#define private public
#include "NiTE.h"
#undef private

#define R_S_NOT_STARTED 4
#define R_E_OPENNI_NO_DEVICES 1
#define R_E_OPENNI_FAILURE 2
#define R_E_NITE_FAILURE 3

/* Not exported */

class KinectV1_NiTEHandler : public ktvr::K2TrackingDeviceBase_KinectBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	KinectV1_NiTEHandler()
	{
		LOG(INFO) << "Constructing the Kinect V1 (X360 - NiTE) Handler for KinectBasis K2TrackingDevice...";

		deviceType = ktvr::K2_Kinect;
		deviceName = "Xbox 360 Kinect (NiTE)";

		deviceCharacteristics = ktvr::K2_Character_Full;
		flipSupported = false; // NiTE has its own flip!
		appOrientationSupported = false; // Sadge
	}
	
	nite::UserTrackerFrameRef mUserFrame;
	nite::UserTracker mUserTracker;
	openni::Device mDevice;

	// Placeholder status: not started
	int32_t status = R_S_NOT_STARTED;
	
	void initialize() override;
	void update() override;
	void shutdown() override;

	~KinectV1_NiTEHandler() override
	{
	}

	HRESULT getStatusResult() override;
	std::wstring statusResultWString(HRESULT stat) override;

private:
	bool initKinect();
	bool initializedOnce = false;

	/* For translating OpenNI joint enumeration to K2 space */
	int globalIndex[25] = {0, 1, 8, 2, 4, 6, 6, 6, 6, 3, 5, 7, 7, 7, 7, 8, 8, 9, 11, 13, 13, 10, 12, 14, 14};
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Devices_Version, pVersionName))
	{
		static KinectV1_NiTEHandler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
