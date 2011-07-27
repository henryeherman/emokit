##################################################
#                epoc daemon                   #
##################################################

add_executable (epoc_svr epoc_svr.c)
ADD_DEPENDENCIES(epoc_svr epoc)
target_link_libraries (epoc_svr epoc ${LIBS})

SET_TARGET_PROPERTIES(epoc_svr PROPERTIES
	INSTALL_RPATH_USE_LINK_PATH TRUE
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	BUILD_WITH_INSTALL_RPATH TRUE )

INSTALL (TARGETS epoc_svr
	RUNTIME DESTINATION bin
)

