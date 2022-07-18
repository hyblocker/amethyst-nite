#include "pch.h"
#include "KinectV1_NiTEHandler.h"
#include "LocalizedStatuses.h"

HRESULT KinectV1_NiTEHandler::getStatusResult()
{
	return status;
}

std::wstring KinectV1_NiTEHandler::statusResultWString(HRESULT stat)
{
	// Wrap status to string for readability
	switch (stat)
	{
	case S_OK: return GetLocalizedStatusWStringAutomatic(kinect::status_ok_map);
	case R_E_OPENNI_NO_DEVICES: return GetLocalizedStatusWStringAutomatic(kinect::status_not_connected_map);
	case R_E_OPENNI_FAILURE: return GetLocalizedStatusWStringAutomatic(nite::status_openni_fail_map);
	case R_E_NITE_FAILURE: return GetLocalizedStatusWStringAutomatic(nite::status_nite_fail_map);
	case R_S_NOT_STARTED: return GetLocalizedStatusWStringAutomatic(nite::status_not_started_map);
	default: return L"Undefined: " + std::to_wstring(stat) +
			L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
	}
}

void KinectV1_NiTEHandler::initialize()
{
	try
	{
		[&, this]
		{
			__try
			{
				initialized = initKinect();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&, this]
				{
					LOG(INFO) <<
						"Initializing: OpenNI/NiTE initialization exception!";
				}();
			}
		}();

		LOG(INFO) << "Initializing: OpenNi/KinectV1 status updated to: " <<
			WStringToString(statusResultWString(getStatusResult()));

		if (!initialized) throw FailedKinectInitialization;
	}
	catch (std::exception& e)
	{
	}
}

void KinectV1_NiTEHandler::shutdown()
{
	try
	{
		LOG(INFO) << "Shutting down: OpenNi/KinectV1 termination pending...";
		status = R_S_NOT_STARTED;

		// Shut down the sensor (Only API)
		[&, this]
		{
			__try
			{
				mUserTracker.destroy();
				mDevice.close();

				openni::OpenNI::shutdown();
				nite::NiTE::shutdown();

				[&, this]
				{
					mUserTracker = nite::UserTracker();
				}();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&, this]
				{
					LOG(INFO) <<
						"Shutting down: OpenNi/KinectV1 termination failed! The device may misbehave until the next reconnection.";
				}();
			}
		}();
	}
	catch (std::exception& e)
	{
		LOG(INFO) <<
			"Shutting down: OpenNi/KinectV1 termination failed! The device may misbehave until the next reconnection.";
	}
}

void KinectV1_NiTEHandler::update()
{
	[&, this]
	{
		__try
		{
			[&, this]
			{
				if (isInitialized() && status == S_OK)
					if (mUserTracker.m_userTrackerHandle != nullptr)
					{
						mUserTracker.readFrame(&mUserFrame);

						const nite::Array<nite::UserData>& aUsers = mUserFrame.getUsers();
						// Ensure that there is at least one user in frame
						if (aUsers.getSize())
						{
							// Select the first user
							const nite::UserData& rUser = aUsers[0];

							if (rUser.isNew())
							{
								// Start tracking for new user
								mUserTracker.startSkeletonTracking(rUser.getId());
							}

							if (rUser.isVisible())
							{
								// This is kinda broken so it's called as freq as possible
								// (and even with that it's not gonna work *always*)
								mUserTracker.setSkeletonSmoothingFactor(.3f);

								// Get user skeleton
								const nite::Skeleton& rSkeleton = rUser.getSkeleton();
								if (rSkeleton.getState() == nite::SKELETON_TRACKED)
								{
									skeletonTracked = true; // We've got it!

									// Get the currently tracked NiTE skeleton
									std::array<nite::SkeletonJoint, 15> a_joints{};
									for (uint32_t i = 0; i <= nite::JOINT_RIGHT_FOOT; i++)
										a_joints[i] = rSkeleton.getJoint(static_cast<nite::JointType>(i));

									/* Copy joint positions */
									for (int j = 0; j < ktvr::Joint_Total; ++j)
									{
										//TrackingDeviceBase::jointPositions[j].w() = skeletonFrame.SkeletonData[i].SkeletonPositions[globalIndex[j]].w;
										auto pos = a_joints[globalIndex[j]].getPosition();
										auto rotQuat = a_joints[globalIndex[j]].getOrientation();

										jointPositions[j].x() = pos.x / 1000.f;
										jointPositions[j].y() = pos.y / 1000.f;
										jointPositions[j].z() = pos.z / 1000.f;

										jointOrientations[j].w() = rotQuat.w;
										jointOrientations[j].x() = rotQuat.x;
										jointOrientations[j].y() = rotQuat.y;
										jointOrientations[j].z() = rotQuat.z;

										trackingStates[j] = a_joints[globalIndex[j]].getPositionConfidence() > 0.5f
											                    ? ktvr::ITrackedJointState::State_Tracked
											                    : ktvr::ITrackedJointState::State_Inferred;
									}

									// Estimate a better chest joint to make the skeleton look better in the preview
									jointPositions[ktvr::Joint_SpineShoulder] = (jointPositions[
										ktvr::Joint_ShoulderLeft] + jointPositions[ktvr::Joint_ShoulderRight]) / 2.f;

									// Estimate a better hip joint
									jointPositions[ktvr::Joint_SpineWaist] = (jointPositions[ktvr::Joint_HipLeft] +
										jointPositions[ktvr::Joint_HipRight]) / 2.f;

									return;
								}
								skeletonTracked = false;
							}
						}
					}
			}();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			[&, this]
			{
				LOG(INFO) <<
					"Updating: OpenNi/KinectV1 loop update failed! The device may misbehave.";

				skeletonTracked = false; // Not tracked
			}();
		}
	}();
}

bool KinectV1_NiTEHandler::initKinect()
{
	// Initialize OpenNI
	openni::OpenNI::initialize();

	// Initialize NiTE
	nite::NiTE::initialize();

	// Open Device
	openni::Array<openni::DeviceInfo> deviceInfoList;
	openni::OpenNI::enumerateDevices(&deviceInfoList);
	int deviceCount = deviceInfoList.getSize();
	LOG(INFO) << "OpenNi/KinectV1 devices found: " << std::to_string(deviceCount);

	// Get a working Kinect Sensor
	if (deviceCount < 1)
	{
		openni::OpenNI::shutdown();
		status = R_E_OPENNI_NO_DEVICES; // No devices
		skeletonTracked = false; // Not tracked
		return false;
	}

	// Just skip everything if we're still good
	// (this will work only for the reconnects)
	if (status == S_OK)
		return true;

	if (initializedOnce)
		return false; // We can't do anything more

	// This is the point of no re-run :/
	initializedOnce = true;

	// Open the device
	if (mDevice.open(deviceInfoList[0].getUri()) != openni::Status::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		status = R_E_OPENNI_FAILURE; // Fail
		skeletonTracked = false; // Not tracked
		return false;
	}

	// Reset the userTracker
	mUserTracker.destroy();

	// Create user tracker
	if (mUserTracker.create(&mDevice) != nite::Status::STATUS_OK)
	{
		openni::OpenNI::shutdown();
		nite::NiTE::shutdown();
		status = R_E_NITE_FAILURE;
		skeletonTracked = false; // Not tracked
		return false;
	}

	status = S_OK; // OK
	return true;
}
