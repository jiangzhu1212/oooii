=== The Ouroboros Libraries ===

This is the foundation library for the coding efforts at OOOii.
(http://www.oooii.com)

The libraries contains fundamental interfaces for typical efforts in advanced 
video and 3D graphics as well as high throughput and multi-threaded programming. 
OOOii periodically refreshes this from time to time, usually after a project-
related QA push. This code is in active development so there are bugs, APIs and 
semantics may change and extensive HTML documentation is still on the yet-to-do 
list.

Our understanding is that all source code distributed under external are legally 
allowed to be used in this library. The intent of distribution is primarily to
ensure a compatible build project and that access to the particular version we
use is available. If a reader finds anything inappropriate with the usage or 
license of external software please contact the author at 
antony.arciuolo@oooii.com or arciuolo@gmail.com.

All comments, feedback and recommendations on better implementations are welcome 
at either antony.arciuolo@oooii.com or arciuolo@gmail.com.

Enjoy!


=== Directory Structure ===

SDK is the development root of all OOOii development. Development is organized 
into suites: logical groupings of several libraries and/or applications. This 
directory can be retrieved in code using the oSYSPATH_DEV value found in 
oSystem.h. The directory should be maintained as follows:

./          <== At the root level all product suites are kept in separate 
                folders (i.e. oooii Ouroboros etc.)
./bin       <== Binaries (DLLs, EXEs) created by all suites are centralized 
                here
./Data      <== All data used by tests and for publishing are stored here
./Ouroboros <== Source code approved by the Principal Architect for open 
                source distribution. Get approval before modifying code in 
                here. If a new module or function, prefer adding code to 
                ./oooii\Source\oStaging until a code review can be done.

At OOOii a suite is defined as a set of libraries, binaries and tools grouped
around a common concept. The Ouroboros suite follows the suite pattern:

<Suite>
./External <== All external/third party libs go here as close to their 
               original downloaded form as possible.
./Include  <== All includes go here and this is the path to put in build 
               settings.
./lib      <== Libraries both external and internal go here.
./Source   <== Source code and build settings/scripts for the libs/exes that 
               comprise this suite go here.
./Scripts  <== Anything needed to deployment on a target machine, like helper 
               batch files. 

Ouroboros is comprised of (roughly in order of dependency):

oStd              <== Implementations of C++11 std namespace objects not yet 
                      supported by Visual Studio. Specifically oStd::future 
                      work-steals, something even gcc's version does not yet 
                      support.
oStdTests         <== Unit tests for oStd.
oHLSL             <== Implementation of the HLSL language for C++. Many of the 
                      math functions in HLSL and utils written for shader code 
                      are useful at tool-time or for C++ code so enable cross-
                      compilation.
oBase             <== Very common utility functions especially relating to 
                      string and memory parsing.
oBaseTests        <== Unit tests for oBase
oSurface          <== Texture/advanced image support including image loading,
                      format introspection, utilities for dealing with 2D and 3D 
                      dimensions as well as mips, arrays, slices and cube maps.
                      this also includes simple fill and conversion utilities.
oSurfaceTests     <== Unit tests for oSurface
oCompute          <== Semi-basic math functions that cross-compile on compute 
                      languages. At the moment this is only HLSL and C++.
oComputeTests     <== Unit tests for oCompute.
oConcurrency      <== Implementation of concurrency containers and sync objects
                      for either bring-up, debugging, or building more complex
                      systems.
oConcurrencyTests <== Unit tests for oConcurrency.
oCore             <== platform/OS concepts like files, debugger, adapters, cpus.
oBasis            <== Basic algorithms, types and structures that have no platform 
                      dependencies (but perhaps lightweight compiler dependencies).
oBasisTests       <== Unit tests for oBasis.
oPlatform         <== Interfaces and implementation for platform-specific objects.
oPlatformTests    <== Unit tets for oPlatform.
oGPU              <== Graphics/3D/Compute is large enough and specialized enough 
                      that it warrants its own lib separate from oPlatform.
oGPUTests         <== Set of simple unit tests for oGPU objects.
oKinect           <== Platform-level integration/wrapper for the Kinect API along
                      with platform-specific visualization utils. Since this is 
                      uncommon specialized hardware, it has been separated from 
                      oPlatform.
oFramework        <== Cross-platform layer that implements complex concepts 
                      generically using oPlatform interfaces, but is not platform
                      -dependent.

NOTE: As of 6/21/2013, oFramework does not exist. Its contents are still inside 
oPlatform as we build more complex ideas around our solidifying platform layer.


=== Required SDKs ===

The project is by default set to build against the Kinect for Windows SDK v1.7.0,
but it seems the license does not specify that we can redistribute it, so either
download and install that SDK or remove the definition for oHAS_KINECT_SDK in 
oKinect.vcproj. The code is set up such that project dependencies can remain and
linking to oKinect.lib should be fine/a noop but if any Kinect-specific APIs are
called outside of the oKinect wrapper class (some util functions) then dealing
with oHAS_KINECT_SDK will need to be done case-by-case.


=== OOOii Gesture System ===

The "gesture system" currently in open source code (a separate one exists 
internally to OOOii) is actually the layering of several different components.
Basically boxes are defined at fixed offsets from bones and named with a 
keyboard key. As many boxes as desired can be defined to create an "air 
keyboard" (see oAirKeyboard). This system gets pumped through the regular 
keyboard system. Key up means a bone is outside the box. Key down means a bone 
is inside the box. Now that motion is tracked as if from pressing keys, the 
gesture system can be treated like a fighting game's combo system and tested 
separately from requiring the physical space Kinect requires. This also allows
for logic debugging using a keyboard separate from environment condition and
Kinect accuracy/precision issues. oInputMapper abstracts the combo system and
should be recognizeable for those who are familiar with setting up MAME 
controls for various exotic arcade input systems: first the input device is 
mapped to a regular keyboard and/or analog axes of a mouse or joystick, then 
various key combinations can be defined to issue a higher-level combo command,
then there are palettes of these combos for different contexts.


=== Refactor Notes ===


Coding Convention
Ouroboros was started before C++11 seemed like a real possibility, so a 
Microsoft-leaning gamer's coding standard was adopted. Now with the rise of 
the mobile programmer, the ascent of Boost and the swelling of stable C++11 
implementations it seems that more code can be made super-generic. Such 
elements in Ouroboros are being migrated to oStd as well as a more std:: or
Boost-looking coding convention. This code may one day not be a part of 
Ouroboros and instead be separated out into its own suite since we hope the
code we present there is applicable across any discipline, not just 
interactive visualization. 

Error Reporting/Exceptions
Since C++11 now defines a robust set of error values as well as robust 
foundations for building user exceptions, oStd code also favors throwing 
rather than return-success and set-last-error. There are simply certain C++ 
standard APIs that make avoiding exception handling impossible (std::future) 
so it is oStd's proposal that exceptions be embraced as the go-to 
exceptional-case handling mechanism.

Reference Counting
Slowly Ouroboros will be migrating towards std::shared_ptr rather than an 
intrusively ref-counted monolithic object model. oInterface will not be 
moved to oStd because it asks too much policy to be adopted while 
std::shared_ptr allows flexibility and defines a standard factory 
interface with make_shared. oMyObjectCreate will likewise be migrating to
std::make_shared over time. Since oInterface also raises the issue of 
QueryInterface, this transition will happen slowly and first on objects
that have little to no polymorphism.

Reflection
Some coding conventions such as no virtual destructor on the base class 
for a true-pure interface and the use of QueryInterface as a substitution 
for multiple inheritance were in preparation for a compiler-based 
implementation of introspects/reflection. Project needs resulted in the
execution of oRTTI, a preprocessor/manual version, but it seems 
sufficient for now and many AAA titles have shipped with such a system,
so other coding conventions may be relaxed for more objects since the
design constraints have also been relaxed.
