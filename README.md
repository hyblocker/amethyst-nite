# KinectNiTE Amethyst device (plugin)

## **License**
This project is licensed under the MIT License. 

## **Downloads**
Uhhhhhhhhhhhhhhhh coming soon&trade;!!

## **Build & Deploy**
Both build and deployment instructions [are available here](https://github.com/hyblocker/amethyst-nite/blob/main/BUILD_AND_DEPLOY.md).

## **Overview**
This repo is an implementation of [OpenNI](https://structure.io/openni)/[PrimeSense](https://en.wikipedia.org/wiki/PrimeSense) as a wrapped [Amethyst](https://github.com/KinectToVR/Amethyst-Releases) plugin.  
Please thank the original devs. [I've used this modified version](https://github.com/korejan/OpenNI2/tree/kinect2), please thank [korejan](https://github.com/korejan).

## **Wanna make one too? (K2API Devices Docs)**
[This repository](https://github.com/KinectToVR/K2TrackingDevice-Samples) contains sample projects of devices / plugins in terms of `Amethyst` project.  
Each project in the solution is an example of a different possible implementation of a tracking device (Later referred to as `K2TrackingDevice`), and an additional one with settings.

<ins>[You can find a detailed description about every single plugin and device type here.](https://github.com/KinectToVR/K2TrackingDevice-Samples/blob/main/DEVICES.md)</ins>

Briefly, currently supported device types are:
- Tracking providers:
  - `JointsBasis` - Provide a vector of named, tracked joints
  - `KinectBasis` - Provide a fixed array of enumerated, tracked joints
    + `Full` Character - Provide Joint_Total joints (as many as Kinect V2 does)
    + `Simple` Character - Provide [ Head, Elbows, Waist, Knees, Ankles, Foot Tips ]
    + `Basic` Character - Provide [ Head, Waist, Ankles ]
- `Spectator` - Only poll data from the app

Please note that `KinectBasis` types can use automatic calibration, whereas `JointsBasis` cannot.  
Additionally, plugins (devices) can provide their own settings, as [described here](https://github.com/KinectToVR/K2TrackingDevice-Samples/blob/main/DEVICES.md#device-settings) and [shown here](https://github.com/KinectToVR/K2TrackingDevice-Samples/blob/main/device_KinectBasis_Full_Settings/DeviceHandler.h#L28).