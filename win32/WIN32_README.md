#Notes for using OWL on Windows

## OWL Version 0.22 or Greater Required
OWL was first ported to windows in version 0.22. Additionally,
it is best to grab a release version, since normal development
is done under Linux.

## LLVM and Clang Required
OWL requires LLVM and libclang. As such, both must be installed.
They can both be obtained as prebuilt installers from the LLVM website.

OWL has been tested with LLVM 3.4, 3.5 and 3.6beta; with the associated
versions of clang.

OWL forwards the responsibility of linking onto clang. As such, clang
must be available and in the PATH environment variable.

## libclang required
Unfortunately, some symbols required by WLC are internal to libclang.
These particular symbols are used for macro parsing 
(specifically clang::cxcursor::getCursorMacroDefinition, 
and clang::cxindex::getMacroInfo)
Because of this, the source code of libclang must be supplied to build WLC on
Windows.

To do so, copy the 'tools\libclang' directory from the clang source code to
the win32 folder of wlc.

## Visual Studios Required
OWL requires Visual Studio to be installed. This is mainly for the 
linker (required by clang) and the Window's link libraries provided
by Visual Studios. OWL has been tested with Visual Studios 2013.

### Libraries Required In LIB Environment Variable
The following directories must be in the 'LIB' environment variable:

* C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86
* C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib

This will allow clang to find the required libraries for building for
Windows. The above directories are for Windows 8.1, and may be different
on other versions.

## Watch out for 'link.exe' conflicts
If MINGW or GNUWIN32 is installed, make sure that the Visual Studio's
'bin' directory has a higher priority in the PATH environment. 
This is to ensure that clang will use VS's link.exe as intended.

## Make sure the correct character set is specified
The 'Use Multi-Byte Character' option must be set as 'Not Set'.
This forces TCHAR to act as char; Without this option, Visual Studios
may complain about a missing winMain function.

## Compiling OWL
To compile OWL, load all source and header files into a visual studios
project. Apart from the default libraries, OWL also requires the LLVM
and clang libraries to build correctly.

under the "Project Properties > Linker > Input > Additional Dependencies"
field, add all .lib and .imp files found in the LLVM lib directory.

Additionally, any .dll's associated with the .imp files 
(such as libclang.dll) should be found in the PATH.

## Additional Libraries
If using Additional libraries on Windows, use the Visual Studios version of the 
library instead of the MinGW Version. Additionally, use the x86 version 
instead of the x64 version.
