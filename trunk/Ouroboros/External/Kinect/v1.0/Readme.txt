The Kinect SDK is not distributed as part of Ouroboros. To build against the Kinect SDK:

1. Download a 1.0 build from http://www.microsoft.com/en-us/kinectforwindows/develop/overview.aspx (version 1.0.3.191 (2 May 2012) build).

2. Copy <Program Files>/Microsoft SDKs/Kinect/v1.0/inc to SDK/Ouroboros/External/Kinect/1.0/inc and <Program Files>/Microsoft SDKs/Kinect/v1.0/lib to SDK/Ouroboros/External/Kinect/1.0/lib.

3. Open properties for oPlatform.vcproj. Go to Configuration Properties|C/C++|Preprocessor and add oHAS_KINECT_SDK as a define.

3. In SDK/Ouroboros/Source/Build/<compiler_version>/Properties/OuroborosPrivateExternalDependencies.props add oHAS_KINECT_SDK under Configuration Properties|C/C++|Preprocessor Defines

4. Recompile oPlatform.