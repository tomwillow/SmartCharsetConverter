set(
	UCHARDET_HEADERS
	uchardet.h
)

file(
	GLOB
	UCHARDET_SOURCES
	LangModels/*.cpp
	CharDistribution.cpp
	JpCntx.cpp
	nsBig5Prober.cpp
	nsCharSetProber.cpp
	nsEscCharsetProber.cpp
	nsEscSM.cpp
	nsEUCJPProber.cpp
	nsEUCKRProber.cpp
	nsEUCTWProber.cpp
#	nsGB2312Prober.cpp
	nsGB18030Prober.cpp
	nsHebrewProber.cpp
	nsLatin1Prober.cpp
	nsMBCSGroupProber.cpp
	nsMBCSSM.cpp
	nsSBCharSetProber.cpp
	nsSBCSGroupProber.cpp
	nsSJISProber.cpp
	nsUniversalDetector.cpp
	nsUTF8Prober.cpp
	uchardet.cpp
)

set (UCHARDET_LIBRARY libuchardet)
set (UCHARDET_LIBRARY libuchardet PARENT_SCOPE)

if (BUILD_STATIC AND BUILD_SHARED_LIBS)
	set (UCHARDET_STATIC_LIBRARY libuchardet_static)
endif ()

add_definitions(
	-DVERSION="${UCHARDET_VERSION}"
	-Wall
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(
		-O0
		-g3
	)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

add_library(
	${UCHARDET_LIBRARY}
	${UCHARDET_SOURCES}
)
target_compile_definitions("${UCHARDET_LIBRARY}" PRIVATE BUILDING_UCHARDET)
if(BUILD_SHARED_LIBS)
	target_compile_definitions("${UCHARDET_LIBRARY}" PUBLIC UCHARDET_SHARED)
endif()

target_include_directories(${UCHARDET_LIBRARY}
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME}>"
)

if (UCHARDET_STATIC_LIBRARY)
	add_library(
		${UCHARDET_STATIC_LIBRARY}
		STATIC
		${UCHARDET_SOURCES}
	)
	target_compile_definitions("${UCHARDET_STATIC_LIBRARY}" PRIVATE BUILDING_UCHARDET)

	target_include_directories(${UCHARDET_STATIC_LIBRARY}
		PUBLIC
			"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
			"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME}>"
	)
endif (UCHARDET_STATIC_LIBRARY)

set_target_properties(
	${UCHARDET_LIBRARY}
	PROPERTIES
		LINKER_LANGUAGE
			CXX
		OUTPUT_NAME
			${PACKAGE_NAME}
		VERSION
			${UCHARDET_VERSION}
		SOVERSION
			${UCHARDET_VERSION_MAJOR}
)

if (UCHARDET_STATIC_LIBRARY)
	set_target_properties(
		${UCHARDET_STATIC_LIBRARY}
		PROPERTIES
			LINKER_LANGUAGE
				CXX
			OUTPUT_NAME
				${PACKAGE_NAME}
	)
endif (UCHARDET_STATIC_LIBRARY)

if (NOT WIN32)
	install(
		TARGETS
			${UCHARDET_LIBRARY}
		EXPORT
			UchardetTargets
		LIBRARY DESTINATION
			${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION
			${CMAKE_INSTALL_LIBDIR}
	)
else (NOT WIN32)
	install(
		TARGETS
			${UCHARDET_LIBRARY}
		EXPORT
			UchardetTargets
		RUNTIME DESTINATION
			${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION
			${CMAKE_INSTALL_LIBDIR}
	)
endif (NOT WIN32)

if (UCHARDET_STATIC_LIBRARY)
	install(
		TARGETS
			${UCHARDET_STATIC_LIBRARY}
		EXPORT
			UchardetTargets
		ARCHIVE DESTINATION
			${CMAKE_INSTALL_LIBDIR}
	)
endif (UCHARDET_STATIC_LIBRARY)

install(
	FILES
		${UCHARDET_HEADERS}
	DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME}
)

include(symbols.cmake)

if (BUILD_BINARY)
	add_subdirectory(tools)
endif (BUILD_BINARY)
