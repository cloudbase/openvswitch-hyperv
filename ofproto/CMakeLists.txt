include_directories("${OVS_Port_SOURCE_DIR}")
include_directories("${OVS_Port_SOURCE_DIR}/include")
include_directories("${OVS_Port_SOURCE_DIR}/lib")
include_directories("${OVS_Port_SOURCE_DIR}/ofproto")
include_directories("${OVS_Port_SOURCE_DIR}/ovsdb")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set (flags_ofproto "-Wstrict-prototypes -Wall -Wno-sign-compare -Wpointer-arith -Wdeclaration-after-statement -Wformat-security -Wswitch-enum -Wunused-parameter -Wstrict-aliasing -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-field-initializers -Wno-override-init  -g")
set( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  ${flags_ofproto}" )
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> cru <LINK_FLAGS> <TARGET> <OBJECTS>")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
include_directories("${OVS_Port_SOURCE_DIR}/windows")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set (sources_ofproto  collectors.c connmgr.c
fail-open.c
in-band.c
names.c netflow.c
ofproto-dpif-governor.c ofproto-dpif.c ofproto-dpif-sflow.c ofproto.c
pinsched.c pktbuf.c)
add_library (ofproto ${sources_ofproto})
