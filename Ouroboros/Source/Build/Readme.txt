This readme describes the contents of the Build directory for Ouroboros.

== Copyright ==
The copyright used for Ouroboros: the open-source portion of OOOii code. NOT ALL CODE IS OPEN SOURCE. http://code.google.com/p/oooii/ is the open source repository.

== msvs ==
Microsoft Visual Studio support. Visual Studio is quite a powerful tool, but not always so well documented. This directory contains various tricks and tools useful for developing with OOOii lib. The tools can be used directly, or as an example for your own codebase.
In Properties, all common Property sheets for use with Ouroboros are stored. The intent that if a project builds using Ouroboros it should replicate the use of these property sheets throughout the project for consistent building.

=== JeffWilcox.VisualStudioGuides.vsix ===
This plugin for VS2010 enables a registry setting to draw a vertical line in the text editor to indicate where coders should manually wrap their code lines for more readable code. HAVE VS CLOSED WHEN YOU DOUBLE-CLICK THIS TO INSTALL! After install, the guide line is configurable by adding/editing a "Guides" string entry at: [HKEY_CURRENT_USER\Software\Microsoft\VisualStudio\10.0\Text Editor]. The format is: RGB(r,g,b) col,col,col. Where r,g,b is each a 0-255 value for the color of the line and the vertical line will be drawn at each col specified. There is a registry script for the standard 80 columns, called JeffWilcox.VisualStudioGuides.80.reg

Yes VisualAssist has this features, but this is free. Read more here: http://www.jeff.wilcox.name/2010/02/visualstudio2010guides/

=== o[En|Dis]ableBSODonCtrl-ScrLkScrLk ===
If you double-click on the oEnableBSODonCtrl-ScrLkScrLk.reg script to install some settings in your registry and reboot your machine, you can cause a user-generated BSOD for testing purposes by holding the RIGHT CTRL (left doesn't do it) and pressing ScrLk twice. This is confirmed working with a USB Keyboard on Windows 7. Use oDisableBSODonCtrl-ScrLkScrLk.reg to turn that key combination off.

=== oFilterDebugOutput ===
Given a text file of entries, this will create a new text file of entries with lines filtered out according to the following ruleset:

oFilterDebugOutput srcfile -i iregex -e eregex [-e eregex -i iregex ...]

o There can be as many filters as desired
o Filters override each other from right to left, so the leftmost filter is the weakest. The idea is to be able to exclude all lines with a word like "leak" in them, but then include that one line with "leak in my system-of-interest" or something like that.
o iregex's if matched against a line will make that line included unless the iregex is overridden by a later exclusion
o eregex's if matched against a line will make that line excluded skipped in writing the new file unless the eregex is overridden by a later inclusion

=== oKillMSBuildProcesses ===
When doing multi-process development, MSBuild.exe's can build up if a process is stopped at the wrong time. So here's a script to kill all those "other" processes that keep referecnes to files, sockets and VirtualAlloc's.

=== oNoStepInto ===
This merges regex's into your registry that are used by MSVC's debugger to prevent stepping into certain functions. This is used for many trivial types or functions that go through a lot of template infrastructure before doing anything meaningful. Once this installs, you'll need to restart Visual Studio for it to take effect. This is automatically installed with the oInstallVSTypes.cmd script.

=== oP4Edit/oP4Revert ===
Perforce Integration into Visual Studio can be cumbersome. It can have stability issues, requires extra files for .scc integration, and doesn't allow for those who don't use the .scc to work gracefully without it. The OOOii solution: Create a custom tool in Visual Studio (Tools|External Tools...) that calls these. Bind them to Alt-R for revert and Alt-C for checkout and you've got 80% of the value of the integration without altering the topology of the project. To auto-install this, merge the VS2010_P4ExtTools.vssettings into Visual Studio using Tools|Import and Export Settings...

=== oVSMacros ===
Contains macros to enhance the Visual Studio experience. Current macros include:
* Alt-O switch-to-header/source like in Visual Assist, but free.

=== usertype.dat ===
This overwrites Visual Studio's usertype.dat file if it exists and users the user-defined keyword color to colorize OOOii lib keywords. Mostly this is used to colorize hlsl keywords so that editing HLSL code in Visual Studio is a bit more consistent with the C++ editing experience. Remember to associated the extension of your shader code files (.fx, .sh) with a C++ editing experience in Tools|Options|Text Editor|File Extension. This is automatically installed with the oInstallVSTypes.cmd script.

== Reporting ==
Scripts to do various statistics gathering.

== SystemStability ==
OOOii writes a lot of "as-console" applications, so here are some registry settings and scripts to help for system stability.

=== oQuickBSODReboot.cmd/.reg ===
The batch file configures Windows for the fastest possible reboot on BSOD, basically disabling:
o a time-taking mem dump write
o the recovery boot screen
o on-boot popups describing errors

In the BuildRules subdirectory there are build rules to use FXC to compile HLSL shaders and oFile2cpp to compile any binary to C++ source code. To use either one of these: right-click on a project that requires shaders, select "Build Customizations" and from the dialog check or browse to find oFXC or oFile2cpp. Once enabled, right-click on an HLSL or binary file in the project for its properties and change the General|Item Type to FXC or oFile2cpp. Another property page option will appear called that will give all the compiler options.
