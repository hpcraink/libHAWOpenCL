# OpenCL Library to aide Programming

This library allows easier handling of OpenCL kernels, providing all current information
on the installed OpenCL platforms, the installed devices and kernels compiled for these.
It is supposed to provide a concise API and good readability for teaching.

Directory [src](src/) contains all source files of the library.
Directory [include](include/) contains the public interface.
Directory [test](test/) contains small tests to print all information (installed as
```bin/opencl_print_info```) and to show how to use this library,
The test opencl_print_info outputs all the OpenCL variables of all platforms
and all supported devices.

## LICENSE
LGPL-2.1 as found in the [LICENSE](LICENSE) file.

## BUILDING
As usual with CMake, out-of-source builds are recommended:
this cleanly separates the sources from any artefacts such as
Makefiles, object files and executables.

### NetBeans:
- In "File->New Project" select "C/C++ Project with Existing Source".
- and "Browse" to the source location.
- (CMake needs to be configured in Your NetBeans Preferences under C/C++)
- In Automatic mode, Netbeans will detect the CMakeLists.txt file
- and build and install the library.

### Unix-Makefiles:
```
   mkdir BUILD && cd BUILD
   cmake ..
   make
```

### Eclipse:
```
   mkdir BUILD && cd BUILD
   cmake -G "Eclipse CDT4 - Unix Makefiles" ..
   Then import in Eclipse.
```

### MacOS XCode:
```
   mkdir BUILD && cd BUILD
   cmake -G Xcode ..
   open HAWOpenCL.xcodeproj
```

ATTENTION: Since MacOS 10.14 OpenCL has been deprecated... This will result in lots of warnings.
Please disable by passing CFLAGS -Wno-deprecated.

## CLEANUP:
If You have run cmake and build IN the main project directory,
then delete all generated files manually using:

```
# Equivalent to a standard make distclean
rm -fr CMakeCache.txt CMakeFiles/ Makefile cmake_install.cmake install_manifest.txt \
       test/CMakeFiles/ test/CTestTestfile.cmake test/cmake_install.cmake test/Makefile \
       src/CMakeFiles/ src/cmake_install.cmake src/Makefile
rm -f include/HAWOpenCL_config.h src/libHAWenCL.a test/opencl_print_info test/opencl_vector_add 
# In case of any BUILD-directories:
rm -fr BUILD*/
# In case of Netbeans:
rm -fr nbproject/
rm -f compile_commands.json
# For MacOS file:
find . -name .DS_Store | xargs rm
```
