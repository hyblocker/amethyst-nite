## **Build instructions**
You'll need:
 - This repo cloned somewhere and `cd`'d into
 - [OpenNI SDK 2.2](https://drive.google.com/file/d/0B3e4_6C5_YOjYmVPQzhwazhBOUE/view?resourcekey=0-0rVbEp0gY1XlZIqEvn5kgg), <ins>PLEASE DON'T INSTALL DRIVERS!</ins>
 - [NiTE SDK 2.2](https://drive.google.com/file/d/0B3e4_6C5_YOjOGIySEluYkNibEE/edit?resourcekey=0-OmTp_BDHirKISLLw3xBTiQ) (Used for skeleton recog & tracking)
 - (For testing purpose) Working installation of Amethyst and SteamVR

Follow these steps:

- [Install tools for the Windows App development](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=vs-2022-17-1-a%2Cvs-2022-17-1-b).<br>
  You'll have to install Visual Studio 2022 or its Build Tools.

- Install [CMake](https://cmake.org/download/) and [git](https://git-scm.com/download/win) if you still somehow don't have them.<br>
  Note: If you have `chocolatey` installed, you can just ```choco install cmake git```

- Set up `GLog` & `GFlags` (For this step you must have cmake installed and visible in PATH)<br>
  ```powershell
  # Download the vswhere tool to find msbuild without additional interactions
  > Invoke-WebRequest -Uri 'https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe' -OutFile './vswhere.exe'
  # Use the downloaded vswhere tool to find msbuild. Skip this step if you are using Developer Powershell
  > $msbuild = "$("$(.\vswhere.exe -legacy -prerelease -products * -format json | Select-String -Pattern "2022" | `
        Select-String -Pattern "Studio" | Select-Object -First 1 | Select-String -Pattern "installationPath")" `
        -replace('"installationPath": "','') -replace('",',''))".Trim() + "\\MSBuild\\Current\\Bin\\MSBuild.exe"

  # Set up external dependencies
  > git submodule update --init

  # Build GLog
  > cd ./external/glog
  > mkdir builds; cd ./builds
  > cmake .. -DBUILD_SHARED_LIBS=ON
  > &"$msbuild" glog.vcxproj "/p:Configuration=Release;Platform=x64"
  # Go back
  > cd ../../..

  # Build GFlags
  > cd ./external/gflags
  > mkdir builds; cd ./builds
  > cmake .. -DBUILD_SHARED_LIBS=ON
  > &"$msbuild" gflags.vcxproj "/p:Configuration=Release;Platform=x64"
  # Go back
  > cd ../../..
  ```

- Build:<br>
  ```powershell
  # Download the vswhere tool to find msbuild without additional interactions
  > Invoke-WebRequest -Uri 'https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe' -OutFile './vswhere.exe'
  # Use the downloaded vswhere tool to find msbuild. Skip this step if you use the Dev Powershell
  > $msbuild = "$("$(.\vswhere.exe -legacy -prerelease -products * -format json | Select-String -Pattern "2022" | `
        Select-String -Pattern "Studio" | Select-Object -First 1 | Select-String -Pattern "installationPath")" `
        -replace('"installationPath": "','') -replace('",',''))".Trim() + "\\MSBuild\\Current\\Bin\\MSBuild.exe"

  # Restore NuGet packages and build everything
  > &"$msbuild" device_KinectNiTE.sln "/p:Configuration=Release;Platform=x64"
  ```

## **Deployment**
The whole output can be found in `x64/Release` directory<br>
(or `x64/Debug` if you're building for `Debug`, naturally) and:
 - Devices (plugins) are inside `devices/` folder

Note: Debug builds will only work with a debug host,<br>
the same schema applies to OpenVR Driver, the API and Amethyst.

## What's inside `device_resources/OpenNI`?
That folder contains OpenNI and NiTE vendor files to make the plugin work.  
The `Drivers` folder contins custom-built drivers from [this repository](https://github.com/KimihikoAkayasaki/OpenNI2).