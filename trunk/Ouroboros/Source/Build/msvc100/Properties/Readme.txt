This describes OOOii lib's usage of MSVC Property Sheets.

One of the more frustrating occurrences in programming is getting 3rd party libs to work together. To better enable this in larger projects, OOOii uses property sheets and ensures all projects built internally use the property sheets so that all build settings remain consistent. When setting up property sheets the rules are:
1. Higher property sheets in the list override those below. vcproj settings are at the highest level.
2. Anything in bold overrides things beneath it. Anything not in bold will take the highest priority setting from the property sheets.

The intended order for OOOii lib is:

OOOiiLib: Overrides some details and gloms together 3rd party libs so any project using OOOii lib does not need to explicitly worry about its dependencies - OOOii.lib will have all the code required. All headers paths of external code that are to be completely encapsulated inside OOOii lib should be put here. If the user must be exposed to the headers, then the header paths should go in Common.

Debug32/64 | Release32/64: These are overrides specified to a debug/release build. Because some featured don't behave in 64-bit the same as they do in 32-bit, there are 4 permutations of build-specifiec overrides.

Msvc90: Anything specific to the compiler/platform is put here. At the time of this writing, it's just a user macro definition.

pch: These are the settings for precompiled headers. The intent is that if pch isn't included, then a regular build is done.

Common: Most of the settings and policy decisions (RTTI or not, exceptions or not, etc.) are done here.

Projects at OOOii have almost no overrides in them at all, except for the occassional retargetting of where outputs are, or additional includes/libs for app-specific requirements.