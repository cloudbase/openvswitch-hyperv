include_directories("${OVS_Port_SOURCE_DIR}")
include_directories("${OVS_Port_SOURCE_DIR}/include")
include_directories("${OVS_Port_SOURCE_DIR}/lib")
include_directories("${OVS_Port_SOURCE_DIR}/ofproto")
include_directories("${OVS_Port_SOURCE_DIR}/ovsdb")
include_directories("${OVS_Port_SOURCE_DIR}/utilities")
include_directories("${OVS_Port_SOURCE_DIR}/tests")
link_directories("${OVS_Port_SOURCE_DIR}")
link_directories("${OVS_Port_SOURCE_DIR}/include")
link_directories("${OVS_Port_SOURCE_DIR}/lib")
link_directories("${OVS_Port_SOURCE_DIR}/ovsdb")
link_directories("${OVS_Port_SOURCE_DIR}/utilities")
link_directories("${OVS_Port_SOURCE_DIR}/tests")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
link_directories("$ENV{PATH}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
include_directories("${OVS_Port_SOURCE_DIR}/windows")
link_directories("${OVS_Port_SOURCE_DIR}/windows")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(flags_ovsdb "-Wstrict-prototypes -Wall -Wno-sign-compare -Wpointer-arith -Wdeclaration-after-statement -Wformat-security -Wswitch-enum -Wunused-parameter -Wstrict-aliasing -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-field-initializers -Wno-override-init -g")
set( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  ${flags_ovsdb}" )
set(CMAKE_EXE_FLAGS "${CMAKE_EXE_FLAGS} -Wstrict-prototypes -Wall -Wno-sign-compare -Wpointer-arith -Wdeclaration-after-statement -Wformat-security -Wswitch-enum -Wunused-parameter -Wstrict-aliasing -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-field-initializers -Wno-override-init -lssl -lcrypto -ldl -lz -lrt -lm")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> cru <LINK_FLAGS> <TARGET> <OBJECTS>")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


set(sources_ovsdb column.c condition.c
execution.c
file.c
jsonrpc-server.c
log.c
mutation.c
ovsdb.c ovsdb-server.c
query.c
row.c
server.c
table.c transaction.c trigger.c)

add_library (ovsdb ${sources_ovsdb})

add_executable (ovsdb-tool ovsdb-tool.c)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (ovsdb-tool ovsdb openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries (ovsdb-tool ovsdb openvswitch win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable (ovsdb-client ovsdb-client.c)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (ovsdb-client ovsdb openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries (ovsdb-client ovsdb openvswitch win_api )# ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable (ovsdb-server ovsdb-server.c)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (ovsdb-server ovsdb openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries (ovsdb-server ovsdb openvswitch win_api )# ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

