# We are requiring 3.10 only because that's the minimum version required by the
# tutorial of the 3.17 release, on which this CMake file is based. We possibly
# could do with a lower version.
cmake_minimum_required ( VERSION 3.10 )

# As a side effect this command sets PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR and
# a few other variables.
# The source directory is the folder where the CMakeLists.txt file resides.
# The binary directory is the folder where CMake is executed.
project (
  # The project name.
  # Sets PROJECT_NAME and, because this is the top-level CMakeLists.txt, also
  # CMAKE_PROJECT_NAME.
  libsgfc++

  # C for most of the SGFC source files
  # CXX (= C++) for the library, unit tests and a couple of SGFC source files
  # C and CXX are the default, so we don't need to set them explicitly.
  LANGUAGES C CXX

  # The project version.
  # Sets PROJECT_VERSION, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR,
  # PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK.
  VERSION 2.0.1

  # Some informational package metadata. Not necessary for the build.
  # Sets PROJECT_HOMEPAGE_URL and PROJECT_DESCRIPTION
  HOMEPAGE_URL "https://github.com/herzbube/libsgfcplusplus"
  DESCRIPTION "A C++ library that uses SGFC to read and write SGF (Smart Game Format) data."
)

set (
  SEPARATOR_LINE "--------------------------------------------------------------------------------"
)
message ( STATUS ${SEPARATOR_LINE} )
message ( STATUS ${PROJECT_NAME} )
message ( STATUS ${PROJECT_DESCRIPTION} )
message ( STATUS ${PROJECT_HOMEPAGE_URL} )
message ( STATUS ${SEPARATOR_LINE} )

# Print some information that may help with identifying build issues
message ( STATUS "CMAKE_VERSION = ${CMAKE_VERSION}" )
message ( STATUS "CMAKE_CXX_COMPILER_ID = ${CMAKE_CXX_COMPILER_ID}" )
message ( STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}" )
message ( STATUS "CMAKE_C_COMPILER_ID = ${CMAKE_C_COMPILER_ID}" )
message ( STATUS "CMAKE_C_COMPILER_VERSION = ${CMAKE_C_COMPILER_VERSION}" )
message ( STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}" )
# On Windows this is the Windows SDK version
message ( STATUS "CMAKE_SYSTEM_VERSION = ${CMAKE_SYSTEM_VERSION}" )
if ( ${MSVC} )
  message ( STATUS "MSVC_VERSION = ${MSVC_VERSION}" )
endif()

# Specify the C++ standard
# We need at least C++17 because of literal initializers like this:
#   const SgfcBoardSize SgfcConstants::BoardSizeMinimum = { 1, 1 };
# Catch2 requires C++11.
set ( CMAKE_CXX_STANDARD 17 )
set ( CMAKE_CXX_STANDARD_REQUIRED True )

# Specify the C standard. This comes from SGFC's own CMakeLists.txt.
set ( CMAKE_C_STANDARD 11 )

# On non-Windows platforms the default is for all symbols to be visible, i.e.
# a library consumer might use an internal class and the linker would not
# complain. We change this default here so that all platforms behave the same.
# Setting the following two variables changes the CMake-wide default so that
# when a new target is added (e.g. with add_library) it will be initialized
# with the proper setting.
set ( CMAKE_CXX_VISIBILITY_PRESET hidden )
set ( CMAKE_VISIBILITY_INLINES_HIDDEN 1 )

# Throwing exceptions in C code, even when compiled for C++, is problematic
# because C does not have exceptions. Compilers on the different platforms
# usually have an option that tells the compiler that C functions (functions
# declared with "extern C") are allowed to throw C++ exceptions. This causes
# the compiler to generate code that allows catching those exceptions. Here we
# define a variable with the compiler-specific option. It is later added to
# each target's compile options.
#
# In a pre-alpha version libsgfc++ did patch SGFC so that some of its functions
# threw a C++ exception. At that time the compiler option was 100% required.
# These days libsgfc++ no longer patches SGFC, instead it installs a
# hook/callback that is invoked by SGFC when an out-of-memory error occurs,
# and the hook/callback throws a C++ exception. So the exception originates
# within a C++ function, but it must flow through many C functions until it
# can be finally caught and handled again in C++ code. It is not clear whether
# the compiler option is still required. For the moment it is left in to be
# on the safe side.
if ( NOT DEFINED COMPILE_OPTION_C_FUNCTIONS_MAY_THROW_EXCEPTIONS )
  if ( ${MSVC} )
    set ( COMPILE_OPTION_C_FUNCTIONS_MAY_THROW_EXCEPTIONS "/EHs" )
  else()
    # We assume that everything else but MSVC is either Clang or GCC. For those
    # the compiler option is the same.
    set ( COMPILE_OPTION_C_FUNCTIONS_MAY_THROW_EXCEPTIONS "-fexceptions" )
  endif()
endif()
message ( STATUS "Compiler option used to specify that C functions may throw exceptions: ${COMPILE_OPTION_C_FUNCTIONS_MAY_THROW_EXCEPTIONS}" )

# Enable testing for this folder and all subfolders. Because this appears in
# the top-level CMakeLists.txt file it enables testing for the entire project.
# CMakeLists.txt in sub-folders can now add tests with the add_test() command.
# We need enable_testing() in this top-level CMakeLists.txt so that CMake
# generates a "test" target (i.e. "make test").
enable_testing ()

# Define a base name that is used throughout the build system to form the
# filesystem name of the build artifacts. Notably this determines the name of
# the generated shared library file, static library file and test executable
# file. The project-specific subdirectory where the public header files are
# installed are also derived from the base name, as well as the CMake package
# name, the exported target names and the macro names related to the export
# header file. The base name should not contain the "++" characters from the
# project name to avoid compiler warnings and unexpected filesystem behaviour.
set ( LIBRARY_BASE_NAME libsgfcplusplus )

# Define target names
set ( SHARED_LIBRARY_TARGET_NAME ${LIBRARY_BASE_NAME}_shared )
set ( STATIC_LIBRARY_TARGET_NAME ${LIBRARY_BASE_NAME}_static )
set ( SHARED_OBJECT_LIBRARY_TARGET_NAME ${SHARED_LIBRARY_TARGET_NAME}_objects )
set ( STATIC_OBJECT_LIBRARY_TARGET_NAME ${STATIC_LIBRARY_TARGET_NAME}_objects )
set ( SHARED_FRAMEWORK_TARGET_NAME ${SHARED_LIBRARY_TARGET_NAME}_framework )
set ( STATIC_FRAMEWORK_TARGET_NAME ${STATIC_LIBRARY_TARGET_NAME}_framework )
set ( TEST_EXECUTABLE_TARGET_NAME ${LIBRARY_BASE_NAME}-test )
set ( EXAMPLE_EXECUTABLE_TARGET_NAME ${LIBRARY_BASE_NAME}-example )
set ( DOCUMENTATION_TARGET_NAME ${LIBRARY_BASE_NAME}-doc )

# Define target output basic names. The regular prefix and suffix will still be
# tacked on.
set ( SHARED_LIBRARY_OUTPUT_NAME ${LIBRARY_BASE_NAME} )
set ( STATIC_LIBRARY_OUTPUT_NAME ${STATIC_LIBRARY_TARGET_NAME} )
set ( SHARED_FRAMEWORK_OUTPUT_NAME ${LIBRARY_BASE_NAME} )
set ( STATIC_FRAMEWORK_OUTPUT_NAME ${STATIC_LIBRARY_TARGET_NAME} )

# On some systems the module GNUInstallDirs is auto-included, but on other
# systems it is not, for unknown reasons. Examples where it's auto-included:
# macOS with CMake 3.11 and Xcode, Windows with CMake 3.16 and VisualStudio.
# Example where it's not included: Ubuntu 18.04 with CMake 3.12 and GCC (the
# Travis build system "bionic"). We manually include the module because our
# install() commands base their destinations on the variables defined by the
# module (e.g. CMAKE_INSTALL_LIBDIR).
if ( NOT DEFINED CMAKE_INSTALL_LIBDIR )
  message ( STATUS "Including module GNUInstallDirs, was not auto-included" )
  include ( GNUInstallDirs )
endif()

# Define where header, library, CMake export files and frameworks will be
# installed to. These must be relative paths.
set ( PUBLIC_HEADER_DESTINATION_FOLDER_NAME ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_BASE_NAME} )
set ( LIBRARY_DESTINATION_FOLDER_NAME ${CMAKE_INSTALL_LIBDIR} )
set ( FRAMEWORK_DESTINATION_FOLDER_NAME "Frameworks" )
# The CMake docs contain two different examples where to put the CMake export
# files: Both install into ${CMAKE_INSTALL_LIBDIR}, but one uses a
# project-specific subfolder (e.g. ${LIBRARY_BASE_NAME}), the other uses the
# constant-named subfolder "cmake". We use the former to reduce the chance of
# filesystem clashes.
set ( EXPORT_DESTINATION_FOLDER_NAME ${CMAKE_INSTALL_LIBDIR}/${LIBRARY_BASE_NAME} )

# The export name that targets should be associated with
set ( EXPORT_NAME ${LIBRARY_BASE_NAME} )
# An export namespace causes the exported target names to be prepended with the
# namespace name. Our targets already have names that include the projectname,
# so are unlikely to cause conflicts in other projects that import our targets.
# However, the CMake docs in the "Importing and Exporting Guide" say that
# providing a namespace with a double-colon suffix (::) is a convention that
# gives CMake a hint that the name is an IMPORTED target and that this can
# result in better diagnostic messages. So let's follow the convention, even
# though it means that downstream projects will have to use longer target names.
set ( EXPORT_NAMESPACE "${LIBRARY_BASE_NAME}::" )

# The name of the CMake package that the project provides.
set ( PACKAGE_NAME ${LIBRARY_BASE_NAME} )
# The base name to use for the package configuration file. The "-config" suffix
# is a convention to ensure that find_package() can find the file based on the
# package name.
set ( PACKAGE_CONFIGURATION_FILE_BASE_NAME "${PACKAGE_NAME}-config" )
# This is the all-important package configuration file that find_package() needs
# to find. No package can exist without a configuration file.
set ( PACKAGE_CONFIGURATION_FILE_NAME "${PACKAGE_CONFIGURATION_FILE_BASE_NAME}.cmake" )
# The package version file accompanies the package configuration file. It's
# optional, but we provide one so that downstream projects can specify an
# acceptable version when they use this project's package. The "-version" suffix
# is another convention that allows find_package() to locate the file.
set ( PACKAGE_VERSION_FILE_NAME "${PACKAGE_CONFIGURATION_FILE_BASE_NAME}-version.cmake" )
# The file to use as a template based on which the package configuration file
# is generated.
set ( PACKAGE_CONFIGURATION_TEMPLATE_FILE_NAME Config.cmake.in )

# Library artifacts generated by a debug configuration build are named
# differently than those generated by release configuration builds.
set ( CMAKE_DEBUG_POSTFIX "d" )

# Every subfolder contains a "sources list" file that must have the following
# file name. The file enumerates the source files to be compiled, and possibly
# other files as well (e.g. public headers). The CMakeLists.txt includes this
# "sources list" file and thus remains relatively succinct. This solution is
# also better than letting CMake make up the file list via globbing, because
# with globbing CMake would not reconfigure when the file list changes in the
# filesystem.
set ( SOURCES_LIST_FILE_NAME "SourcesList.cmake" )

# Define defaults for optional build parts
if ( NOT DEFINED ENABLE_DEFAULT )
  set ( ENABLE_DEFAULT YES )
endif()
if ( NOT DEFINED ENABLE_SHARED_LIBRARY )
  # Unless the user explicitly requests it by setting ENABLE_SHARED_LIBRARY to
  # YES, don't build the shared library for iOS. The reason: The shared library
  # needs codesigning, but the project does not provide a codesigning identity,
  # so the iOS build would fail. We don't want that, so by default we don't
  # build the shared library for iOS.
  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set ( ENABLE_SHARED_LIBRARY ${ENABLE_DEFAULT} )
  else()
    set ( ENABLE_SHARED_LIBRARY NO )
  endif()
endif()
if ( NOT DEFINED ENABLE_STATIC_LIBRARY )
  set ( ENABLE_STATIC_LIBRARY ${ENABLE_DEFAULT} )
endif()
if ( NOT DEFINED ENABLE_SHARED_FRAMEWORK )
  # Unless the user explicitly requests it by setting ENABLE_SHARED_FRAMEWORK to
  # YES, don't build the shared library framework for iOS. For the reason see
  # the comments above why we also don't build the shared library for iOS.
  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set ( ENABLE_SHARED_FRAMEWORK ${ENABLE_DEFAULT} )
  else()
    set ( ENABLE_SHARED_FRAMEWORK NO )
  endif()
endif()
if ( NOT DEFINED ENABLE_STATIC_FRAMEWORK )
  set ( ENABLE_STATIC_FRAMEWORK ${ENABLE_DEFAULT} )
endif()
if ( NOT DEFINED ENABLE_TESTS )
  # Unless the user explicitly requests it by setting ENABLE_TESTS to YES,
  # don't build tests for iOS. The reason: The test runner executable needs
  # codesigning, and for it to be codesigned it would require a bundle
  # identifier. The project neither provides a bundle identifier, nor a
  # codesigning identity, so the iOS build would fail. We don't want that, so
  # by default we don't build tests for iOS.
  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set ( ENABLE_TESTS ${ENABLE_DEFAULT} )
  else()
    set ( ENABLE_TESTS NO )
  endif()
endif()
if ( NOT DEFINED ENABLE_EXAMPLES )
  # Unless the user explicitly requests it by setting ENABLE_EXAMPLES to YES,
  # don't build the examples for iOS. For the reason see the comments above
  # why we also don't build tests for iOS.
  if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set ( ENABLE_EXAMPLES ${ENABLE_DEFAULT} )
  else()
    set ( ENABLE_EXAMPLES NO )
  endif()
endif()
if ( ${ENABLE_SHARED_LIBRARY} )
  message ( STATUS "Will build shared library." )
endif()
if ( ENABLE_SHARED_FRAMEWORK )
  message ( STATUS "Will build shared framework." )
endif()
if ( ENABLE_STATIC_LIBRARY )
  message ( STATUS "Will build static library." )
endif()
if ( ENABLE_STATIC_FRAMEWORK )
  message ( STATUS "Will build static framework." )
endif()
if ( ENABLE_TESTS )
  message ( STATUS "Will build tests." )
endif()
if ( ENABLE_EXAMPLES )
  message ( STATUS "Will build examples." )
endif()

# Set variables that depend on which parts of the build have been enabled
if ( ${ENABLE_SHARED_LIBRARY} OR ${ENABLE_STATIC_LIBRARY} OR ${ENABLE_SHARED_FRAMEWORK} OR ${ENABLE_STATIC_FRAMEWORK} )
  set ( AT_LEAST_ONE_LIBRARY_IS_ENABLED YES )
else()
  set ( AT_LEAST_ONE_LIBRARY_IS_ENABLED NO )
endif()
if ( ${ENABLE_SHARED_LIBRARY} OR ${ENABLE_SHARED_FRAMEWORK} )
  set ( AT_LEAST_ONE_SHARED_LIBRARY_IS_ENABLED YES )
else()
  set ( AT_LEAST_ONE_SHARED_LIBRARY_IS_ENABLED NO )
endif()
if ( ${ENABLE_STATIC_LIBRARY} OR ${ENABLE_STATIC_FRAMEWORK} )
  set ( AT_LEAST_ONE_STATIC_LIBRARY_IS_ENABLED YES )
else()
  set ( AT_LEAST_ONE_STATIC_LIBRARY_IS_ENABLED NO )
endif()

# CMake ties the function to generate an export header file to a target. We
# have four targets but want to generate the header file only once, so here we
# choose a target. It doesn't matter which one we choose because we will
# override the file and preprocessor macro naming.
if ( ${ENABLE_SHARED_LIBRARY} )
  set ( EXPORT_HEADER_TARGET_NAME ${SHARED_LIBRARY_TARGET_NAME} )
elseif ( ENABLE_SHARED_FRAMEWORK )
  set ( EXPORT_HEADER_TARGET_NAME ${SHARED_FRAMEWORK_TARGET_NAME} )
elseif ( ENABLE_STATIC_LIBRARY )
  set ( EXPORT_HEADER_TARGET_NAME ${STATIC_LIBRARY_TARGET_NAME} )
elseif ( ENABLE_STATIC_FRAMEWORK )
  set ( EXPORT_HEADER_TARGET_NAME ${STATIC_FRAMEWORK_TARGET_NAME} )
else()
  message ( FATAL_ERROR "Failed to generate export header, no target found because none of the libraries is being built" )
endif()

# Define variables required to generate and consume the export header file
set ( EXPORT_HEADER_BASE_NAME SGFCPLUSPLUS )
# This is the macro being defined in the export header file and being used
# throughout the library in type declarations to define the types's symbol
# visibility.
set ( EXPORT_HEADER_SYMBOL_VISIBILITY_MACRO_NAME ${EXPORT_HEADER_BASE_NAME}_EXPORT )
# Shared library object files must be compiled with this macro defined
set ( EXPORT_HEADER_EXPORTS_MACRO_NAME ${EXPORT_HEADER_TARGET_NAME}_EXPORTS )
# Static library object files must be compiled with this macro defined.
# Anyone who wants to link against the static library or the static library
# framework must also define this macro. Example: The unit test project.
set ( EXPORT_HEADER_STATIC_DEFINE_MACRO_NAME ${EXPORT_HEADER_BASE_NAME}_STATIC_DEFINE )
message ( STATUS "Linking against the static library requires the consumer to define this preprocessor macro during the build: ${EXPORT_HEADER_STATIC_DEFINE_MACRO_NAME}" )

# Unit tests of course also test library-internal classes. The symbols for these
# internal classes are not visible in shared libraries. The unit test target
# therefore must link against a static library to gain access to all symbols.
if ( ENABLE_STATIC_LIBRARY )
  set ( LINK_LIBRARY_TARGET_NAME ${STATIC_LIBRARY_TARGET_NAME} )
elseif ( ENABLE_STATIC_FRAMEWORK )
  set ( LINK_LIBRARY_TARGET_NAME ${STATIC_FRAMEWORK_TARGET_NAME} )
else()
  set ( LINK_LIBRARY_TARGET_NAME "" )
endif()

# Integrate iconv. This is required by SGFC's encoding/decoding feature.
set ( ICONV_NEEDS_LINKING NO )
set ( ICONV_NEEDS_INCLUDE_DIRECTORY NO )
set ( ICONV_NEEDS_COMPILING NO )
if ( "${UNIX}" )
  if ( "${APPLE}" OR "${CYGWIN}" OR "${MINGW}" )
    # 1) Apple platforms (macOS, iOS, etc.) have an iconv implementation that
    #    lives in a separate library, not in the system's C library. The output
    #    of the cmdline tool "iconv --help" indicates that this is GNU libiconv.
    # 2) Cygwin and MSYS/MINGW are known to use GNU libiconv.
    set ( ICONV_NEEDS_LINKING YES )
    find_library ( ICONV_LIBRARY iconv REQUIRED )
    if ( NOT ICONV_LIBRARY )
      message ( FATAL_ERROR "iconv library not found" )
    endif()
  else()
    # Assume we're on Linux, BSD or some other UNIX/UNIX-like system that has
    # a native iconv implementation (e.g. glibc on Linux). No need to link.
  endif()
elseif ( "${WIN32}" )
  set ( ICONV_NEEDS_INCLUDE_DIRECTORY YES )
  set ( ICONV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/win-iconv )
  set ( ICONV_NEEDS_COMPILING YES )
else()
  # An unexpected/unsupported system type. The user will have to find a way
  # how to make things work.
endif()

# Perform some logic checks
if ( ${ENABLE_TESTS} OR ${ENABLE_EXAMPLES} )
  message ( CHECK_START "Check whether tests and examples can be built" )
  if ( ${AT_LEAST_ONE_STATIC_LIBRARY_IS_ENABLED} )
    message ( CHECK_PASS "yes")
    message ( STATUS "Tests and examples will be linked against target ${LINK_LIBRARY_TARGET_NAME}" )
  else()
    message ( CHECK_FAIL "no: all static libraries have been disabled" )
    message ( FATAL_ERROR "Tests and examples need to be linked against a static library" )
  endif()
endif()

# Add subfolders to the build that must contain other CMakeLists.txt files.
#
# IMPORTANT: These commands should appear at the end of this CMakeLists.txt
# because other commands further up have an impact on what is inside the
# subfolders' CMakeLists.txt files (e.g. setting CMAKE_CXX_STANDARD).
if ( ${AT_LEAST_ONE_LIBRARY_IS_ENABLED} )
  add_subdirectory ( "src" )
endif()
if ( ${ENABLE_TESTS} )
  add_subdirectory ( "test" )
endif()
if ( ${ENABLE_EXAMPLES} )
  add_subdirectory ( "example" )
endif()
add_subdirectory ( "doc" )
