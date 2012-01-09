Visual Studio is quite a powerful tool, but not always so well documented. This directory contains various tricks and tools useful for developing with OOOii lib. The tools can be used directly, or as an example for your own codebase.

== oCustomBuildRules ==
Contains build rules that can be used for specific types. Currently it defines:
* A build rule to turn any file into a .cpp buffer. This is useful for compiling icons or other binaries directly into the exe or lib
* A build rule for building HLSL shader code using fxc.exe to bytecode source headers.

== oVSMacros ==
Contains macros to enhance the Visual Studio experience. Current macros include:
* Alt-O switch-to-header/source like in Visual Assist, but without the $300 fee.

== oNoStepInto ==
This merges regex's into your registry that are used by MSVC's debugger to prevent stepping into certain functions. This is used for many trivial types or functions that go through a lot of template infrastructure before doing anything meaningful. Once this installs, you'll need to restart Visual Studio for it to take effect.

== usertype.dat ==
This overwrites Visual Studio's usertype.dat file if it exists and users the user-defined keyword color to colorize OOOii lib keywords. Mostly this is used to colorize hlsl keywords so that editing HLSL code in Visual Studio is a bit more consistent with the C++ editing experience. Remember to associated the extension of your shader code files (.fx, .sh) with a C++ editing experience in Tools|Options|Text Editor|File Extension.

== o[En|Dis]ableBSODonCtrl-ScrLkScrLk ==
If you double-click on the oEnableBSODonCtrl-ScrLkScrLk.reg script to install some settings in your registry and reboot your machine, you can cause a user-generated BSOD for testing purposes by holding the RIGHT CTRL (left doesn't do it) and pressing ScrLk twice. This is confirmed working with a USB Keyboard on Windows 7. Use oDisableBSODonCtrl-ScrLkScrLk.reg to turn that key combination off.

== oQuickBSODReboot.cmd/.reg ==

The batch file configures Windows for the fastest possible reboot on BSOD, basically disabling:
o a time-taking mem dump write
o the recovery boot screen
o on-boot popups describing errors

== oFilterDebugOutput ==

Given a text file of entries, this will create a new text file of entries with lines filtered out according to the following ruleset:

oFilterDebugOutput srcfile -i iregex -e eregex [-e eregex -i iregex ...]

o There can be as many filters as desired
o Filters override each other from right to left, so the leftmost filter is the weakest. The idea is to be able to exclude all lines with a word like "leak" in them, but then include that one line with "leak in my system-of-interest" or something like that.
o iregex's if matched against a line will make that line included unless the iregex is overridden by a later exclusion
o eregex's if matched against a line will make that line excluded skipped in writing the new file unless the eregex is overridden by a later inclusion