include_directories("${OVS_Port_SOURCE_DIR}")
include_directories("${OVS_Port_SOURCE_DIR}/include")
include_directories("${OVS_Port_SOURCE_DIR}/lib")
include_directories("${OVS_Port_SOURCE_DIR}/ofproto")
include_directories("${OVS_Port_SOURCE_DIR}/ovsdb")
include_directories("${OVS_Port_SOURCE_DIR}/utilities")
link_directories("${OVS_Port_SOURCE_DIR}")
link_directories("${OVS_Port_SOURCE_DIR}/include")
link_directories("${OVS_Port_SOURCE_DIR}/lib")
link_directories("${OVS_Port_SOURCE_DIR}/ovsdb")
link_directories("${OVS_Port_SOURCE_DIR}/utilities")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
link_directories("$ENV{PATH}")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
include_directories("${OVS_Port_SOURCE_DIR}/windows")
link_directories("${OVS_Port_SOURCE_DIR}/windows")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(CMAKE_EXE_FLAGS "${CMAKE_EXE_FLAGS} -Wstrict-prototypes -Wall -Wno-sign-compare -Wpointer-arith -Wdeclaration-after-statement -Wformat-security -Wswitch-enum -Wunused-parameter -Wstrict-aliasing -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-field-initializers -Wno-override-init  -g")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_executable (ovs-vswitchd ovs-vswitchd.c bridge.c system-stats.c xenserver.c)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries (ovs-vswitchd ofproto sflow openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries (ovs-vswitchd ofproto openvswitch sflow win_api)#openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


