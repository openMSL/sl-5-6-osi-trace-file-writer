set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LINK_WITH_SHARED_OSI OFF CACHE BOOL "Link FMU with shared OSI library instead of statically linking")
set(PUBLIC_LOGGING OFF CACHE BOOL "Enable logging via FMI logger")
set(PRIVATE_LOGGING OFF CACHE BOOL "Enable private logging to file")

string(TIMESTAMP FMUTIMESTAMP UTC)
string(MD5 FMUGUID modelDescription.in.xml)
configure_file(modelDescription.in.xml modelDescription.xml @ONLY)

find_package(Protobuf 2.6.1 REQUIRED)
add_library(sl-5-6-osi-trace-file-writer SHARED
		OSMP.cpp
		OSMP.h
		TraceFileWriter.cpp
		TraceFileWriter.h)
set_target_properties(sl-5-6-osi-trace-file-writer PROPERTIES PREFIX "")
target_compile_definitions(sl-5-6-osi-trace-file-writer PRIVATE "FMU_SHARED_OBJECT")
if(LINK_WITH_SHARED_OSI)
	target_link_libraries(sl-5-6-osi-trace-file-writer open_simulation_interface)
else()
	target_link_libraries(sl-5-6-osi-trace-file-writer open_simulation_interface_pic)
endif()

target_link_libraries(sl-5-6-osi-trace-file-writer OSIUtilities)

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(FMI_BINARIES_PLATFORM "win64")
	else()
		set(FMI_BINARIES_PLATFORM "win32")
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(FMI_BINARIES_PLATFORM "linux64")
	else()
		set(FMI_BINARIES_PLATFORM "linux32")
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(FMI_BINARIES_PLATFORM "darwin64")
	else()
		set(FMI_BINARIES_PLATFORM "darwin32")
	endif()
endif()

add_custom_command(TARGET sl-5-6-osi-trace-file-writer
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/buildfmu"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/binaries/${FMI_BINARIES_PLATFORM}"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/modelDescription.xml" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/OSMP.cpp" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources/"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/OSMP.h" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources/"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/TraceFileWriter.cpp" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources/"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/TraceFileWriter.h" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources/"
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sl-5-6-osi-trace-file-writer> $<$<PLATFORM_ID:Windows>:$<$<CONFIG:Debug>:$<TARGET_PDB_FILE:sl-5-6-osi-trace-file-writer>>> "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/binaries/${FMI_BINARIES_PLATFORM}"
		COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}/buildfmu" ${CMAKE_COMMAND} -E tar "cfv" "${FMU_INSTALL_DIR}/sl-5-6-osi-trace-file-writer.fmu" --format=zip "modelDescription.xml" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/sources" "${CMAKE_CURRENT_BINARY_DIR}/buildfmu/binaries/${FMI_BINARIES_PLATFORM}")