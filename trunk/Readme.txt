=== The Ouroboros Libraries ===

This library started as a personal project, then was brought into OOOii as the 
foundation library for the coding efforts there, and has since evolved back into 
a generic foundation library. (http://www.oooii.com)

The libraries contains fundamental interfaces for typical efforts in advanced 
video and 3D graphics as well as high throughput and multi-threaded programming. 
This code is in active development so there are bugs, APIs and semantics may 
change and extensive documentation is still on the yet-to-do list.

My understanding is that all 3rd-party source code distributed with Ouroboros 
code are legally allowed to be distributed. The intent of distribution is 
primarily to ensure a compatible build project and that access to the particular 
version the library use is always available. If a reader finds anything 
inappropriate with the usage or license of external software please contact the 
author at arciuolo@gmail.com.

All comments, feedback and recommendations on better implementations are welcome 
at arciuolo@gmail.com.

Enjoy!


=== Directory Structure ===

A suite is defined as a set of libraries, binaries and tools grouped around a 
common concept. The Ouroboros suite was one of several OOOii libraries, and it 
follows the suite pattern:

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
                      are useful at tool-time or for C++ code so this library 
                      enables cross-compilation.
oBase             <== Very common utility functions especially relating to 
                      string and memory parsing.
oBaseTests        <== Unit tests for oBase.
oSurface          <== Texture/advanced image support including image loading,
                      format introspection, utilities for dealing with 2D and 3D 
                      dimensions as well as mips, arrays, slices and cube maps.
                      this also includes simple fill and conversion utilities.
oSurfaceTests     <== Unit tests for oSurface.
oCompute          <== Semi-basic math functions that cross-compile on compute 
                      languages. At the moment this is only HLSL and C++.
oComputeTests     <== Unit tests for oCompute.
oCore             <== platform/OS concepts like files, debugger, adapters, cpus.
oCoreTests        <== Unit tests for oCore.
oConcurrency      <== Implementation of concurrency containers and sync objects
                      for either bring-up, debugging, or building more complex
                      systems.
oConcurrencyTests <== Unit tests for oConcurrency.
oGUI              <== graphical user interface concepts, often not abstracted or 
                      cross-platform - this isn't intended to be QT or WxWidgets,
                      but just the very basics until it's worth bringing in a 
                      heavyweight middleware.
oGUITests         <== Unit tests for oGUI.
oGPU              <== Interacting with a GPU
oGPUTests         <== Unit tests for oGPU.
oGfx              <== A renderer based on oGPU


LEGACY 
These libraries are scheduled for removal, though their contents will 
shift to one of theabovce libraries

oBasis            <== Basic algorithms, types and structures that have no 
                      platform dependencies (but perhaps lightweight compiler 
                      dependencies).
oBasisTests       <== Unit tests for oBasis.
oPlatform         <== Interfaces and implementation for platform-specific objects.
oPlatformTests    <== Unit tets for oPlatform.
oKinect           <== Platform-level integration/wrapper for the Kinect API along
                      with platform-specific visualization utils. Since this is 
                      uncommon specialized hardware, it has been separated from 
                      oPlatform.

FUTURE
These libraries don't exit yet, but will be a landing pad for things from the
deprecated libraries.

oFramework        <== Cross-platform layer that implements complex concepts 
                      generically using oPlatform interfaces, but is not platform
                      -dependent.

=== Required SDKs ===

The project is by default set to build against the Kinect for Windows SDK v1.7.0,
but it seems the license does not specify that we can redistribute it, so either
download and install that SDK or remove the definition for oHAS_KINECT_SDK in 
oKinect.vcproj. The code is set up such that project dependencies can remain and
linking to oKinect.lib should be fine/a noop but if any Kinect-specific APIs are
called outside of the oKinect wrapper class (some util functions) then dealing
with oHAS_KINECT_SDK will need to be done case-by-case in client code.


=== Ouroboros Gesture System ===

The "gesture system" for use with skeleton tracking devices like Kinect is 
actually the layering of several different components. Basically boxes are 
defined at fixed offsets from bones and named with a keyboard key. As many boxes 
as desired can be defined to create an "air keyboard" (see oAirKeyboard). This 
system gets pumped through the regular keyboard system. Key up means a bone is 
outside the box. Key down means a bone is inside the box. Now that motion is 
tracked as if from pressing keys, the gesture system can be treated like a 
fighting game's combo system and tested separately from requiring the physical 
space Kinect requires. This also allows for logic debugging using a keyboard 
separate from environment condition and Kinect accuracy/precision issues. 
oInputMapper abstracts the combo system and should be recognizeable for those 
who are familiar with setting up MAME controls for various exotic arcade input 
systems: first the input device is mapped to a regular keyboard and/or analog 
axes of a mouse or joystick, then various key combinations can be defined to 
issue a higher-level combo command, then there are palettes of these combos for 
different contexts.


=== Refactor Notes ===

Coding Convention
Ouroboros was started before C++11 seemed like a real possibility, so a 
Microsoft-leaning gamer's coding standard was adopted. Now with the rise of 
the mobile programmer, the ascent of Boost and the swelling of stable C++11 
implementations it seems that more code can be made super-generic. Such 
elements in Ouroboros are being migrated to a more std:: or Boost-looking coding 
convention.

Error Reporting/Exceptions
Since C++11 now defines a robust set of error values as well as robust 
foundations for building user exceptions, newer code favors throwing rather than 
return-success or set-last-error. There are simply certain C++ standard APIs 
that make avoiding exception handling impossible (std::future) so Ouroboros 
embraces exceptions and the primary error handling mechanism.

Reference Counting
Ouroboros is migrating towards std::shared_ptr rather than an intrusively ref-
counted monolithic object model. oInterface is deprecated because it asks too 
much policy to be adopted while std::shared_ptr allows flexibility and defines 
a standard factory interface with make_shared. oMyObjectCreate will likewise be 
migrating to std::make_shared over time. Since oInterface also raises the issue 
of QueryInterface, this transition will happen slowly and first on objects
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
