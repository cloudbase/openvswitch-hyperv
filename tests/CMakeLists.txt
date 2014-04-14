cmake_minimum_required(VERSION 2.6)
enable_testing ()

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
set(CMAKE_EXE_FLAGS "${CMAKE_EXE_FLAGS} -Wstrict-prototypes -Wall -Wno-sign-compare -Wpointer-arith -Wdeclaration-after-statement -Wformat-security -Wswitch-enum -Wunused-parameter -Wstrict-aliasing -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-field-initializers -Wno-override-init -g ")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_executable(test-aes128 test-aes128.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-aes128 openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-aes128 openvswitch win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-bundle test-bundle.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-bundle openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-bundle openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-classifier test-classifier.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-classifier openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-classifier openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-csum test-csum.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-csum openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-csum openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-file_name test-file_name.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-file_name openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-file_name openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-flows test-flows.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-flows openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-flows openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-hash test-hash.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-hash openvswitch rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-hash openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-heap test-heap.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-heap openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-heap openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-hmap test-hmap.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-hmap openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-hmap openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-json test-json.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-json openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-json openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-jsonrpc test-jsonrpc.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-jsonrpc openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-jsonrpc openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-list test-list.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-list openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-list openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-lockfile test-lockfile.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-lockfile openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-lockfile openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-multipath test-multipath.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-multipath openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-multipath openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-packets test-packets.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-packets openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-packets openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-random test-random.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-random openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-random openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-stp test-stp.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-stp openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-stp openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-netflow test-netflow.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-netflow openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-netflow openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-unix-socket test-unix-socket.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-unix-socket openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-unix-socket openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-odp test-odp.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-odp openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-odp openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-ovsdb test-ovsdb.c idltest.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-ovsdb ovsdb openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-ovsdb openvswitch ovsdb win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-reconnect test-reconnect.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-reconnect openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-reconnect openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-sha1 test-sha1.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-sha1 openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-sha1 openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#add_executable(test-timeval test-timeval.c) 
#	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#	target_link_libraries(test-timeval openvswitch ssl crypto dl z rt m)
#	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#	target_link_libraries(test-timeval openvswitch  win_api )#ssl crypto dl z rt m)
#	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-strtok_r test-strtok_r.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-strtok_r rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-strtok_r openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-type-props test-type-props.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-type-props rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-type-props openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-util test-util.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-util openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-util openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-uuid test-uuid.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-uuid openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-uuid openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-vconn test-vconn.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-vconn openvswitch ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-vconn openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(test-byte-order test-byte-order.c) 
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(test-byte-order openvswitch rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(test-byte-order openvswitch  win_api )#ssl crypto dl z rt m)
	ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

########################################
# aes128.at
########################################

add_test(wikipedia_test_vector_1 test-aes128 "00010203050607080a0b0c0d0f101112" "506812a45f08c889b97f5980038b8359")
set_tests_properties (wikipedia_test_vector_1
  PROPERTIES PASS_REGULAR_EXPRESSION "d8f532538289ef7d06b506a4fd5be9c9")

add_test(wikipedia_test_vector_2 test-aes128 "95A8EE8E89979B9EFDCBC6EB9797528D" "4ec137a426dabf8aa0beb8bc0c2b89d6")
set_tests_properties (wikipedia_test_vector_2
  PROPERTIES PASS_REGULAR_EXPRESSION "d9b65d1232ba0199cdbd487b2a1fd646")

add_test(NIST_KAT_ECBKeySbox128e_vector_0 test-aes128 "10a58869d74be5a374cf867cfb473859" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_0
  PROPERTIES PASS_REGULAR_EXPRESSION "6d251e6944b051e04eaa6fb4dbf78465")

add_test(NIST_KAT_ECBKeySbox128e_vector_1 test-aes128 "caea65cdbb75e9169ecd22ebe6e54675" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_1
  PROPERTIES PASS_REGULAR_EXPRESSION "6e29201190152df4ee058139def610bb")

add_test(NIST_KAT_ECBKeySbox128e_vector_2 test-aes128 "a2e2fa9baf7d20822ca9f0542f764a41" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_2
  PROPERTIES PASS_REGULAR_EXPRESSION "c3b44b95d9d2f25670eee9a0de099fa3")

add_test(NIST_KAT_ECBKeySbox128e_vector_3 test-aes128 "b6364ac4e1de1e285eaf144a2415f7a0" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_3
  PROPERTIES PASS_REGULAR_EXPRESSION "5d9b05578fc944b3cf1ccf0e746cd581")

add_test(NIST_KAT_ECBKeySbox128e_vector_4 test-aes128 "64cf9c7abc50b888af65f49d521944b2" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_4
  PROPERTIES PASS_REGULAR_EXPRESSION "f7efc89d5dba578104016ce5ad659c05")

add_test(NIST_KAT_ECBKeySbox128e_vector_5 test-aes128 "47d6742eefcc0465dc96355e851b64d9" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_5
  PROPERTIES PASS_REGULAR_EXPRESSION "0306194f666d183624aa230a8b264ae7")

add_test(NIST_KAT_ECBKeySbox128e_vector_6 test-aes128 "3eb39790678c56bee34bbcdeccf6cdb5" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_6
  PROPERTIES PASS_REGULAR_EXPRESSION "858075d536d79ccee571f7d7204b1f67")

add_test(NIST_KAT_ECBKeySbox128e_vector_7 test-aes128 "64110a924f0743d500ccadae72c13427" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_7
  PROPERTIES PASS_REGULAR_EXPRESSION "35870c6a57e9e92314bcb8087cde72ce")

add_test(NIST_KAT_ECBKeySbox128e_vector_8 test-aes128 "18d8126516f8a12ab1a36d9f04d68e51" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_8
  PROPERTIES PASS_REGULAR_EXPRESSION "6c68e9be5ec41e22c825b7c7affb4363")

add_test(NIST_KAT_ECBKeySbox128e_vector_9 test-aes128 "f530357968578480b398a3c251cd1093" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_9
  PROPERTIES PASS_REGULAR_EXPRESSION "f5df39990fc688f1b07224cc03e86cea")

add_test(NIST_KAT_ECBKeySbox128e_vector_10 test-aes128 "da84367f325d42d601b4326964802e8e" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_10
  PROPERTIES PASS_REGULAR_EXPRESSION "bba071bcb470f8f6586e5d3add18bc66")

add_test(NIST_KAT_ECBKeySbox128e_vector_11 test-aes128 "e37b1c6aa2846f6fdb413f238b089f23" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_11
  PROPERTIES PASS_REGULAR_EXPRESSION "43c9f7e62f5d288bb27aa40ef8fe1ea8")

add_test(NIST_KAT_ECBKeySbox128e_vector_12 test-aes128 "6c002b682483e0cabcc731c253be5674" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_12
  PROPERTIES PASS_REGULAR_EXPRESSION "3580d19cff44f1014a7c966a69059de5")

add_test(NIST_KAT_ECBKeySbox128e_vector_13 test-aes128 "143ae8ed6555aba96110ab58893a8ae1" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_13
  PROPERTIES PASS_REGULAR_EXPRESSION "806da864dd29d48deafbe764f8202aef")

add_test(NIST_KAT_ECBKeySbox128e_vector_14 test-aes128 "b69418a85332240dc82492353956ae0c" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_14
  PROPERTIES PASS_REGULAR_EXPRESSION "a303d940ded8f0baff6f75414cac5243")

add_test(NIST_KAT_ECBKeySbox128e_vector_15 test-aes128 "71b5c08a1993e1362e4d0ce9b22b78d5" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_15
  PROPERTIES PASS_REGULAR_EXPRESSION "c2dabd117f8a3ecabfbb11d12194d9d0")

add_test(NIST_KAT_ECBKeySbox128e_vector_16 test-aes128 "e234cdca2606b81f29408d5f6da21206" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_16
  PROPERTIES PASS_REGULAR_EXPRESSION "fff60a4740086b3b9c56195b98d91a7b")

add_test(NIST_KAT_ECBKeySbox128e_vector_17 test-aes128 "13237c49074a3da078dc1d828bb78c6f" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_17
  PROPERTIES PASS_REGULAR_EXPRESSION "8146a08e2357f0caa30ca8c94d1a0544")

add_test(NIST_KAT_ECBKeySbox128e_vector_18 test-aes128 "3071a2a48fe6cbd04f1a129098e308f8" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_18
  PROPERTIES PASS_REGULAR_EXPRESSION "4b98e06d356deb07ebb824e5713f7be3")

add_test(NIST_KAT_ECBKeySbox128e_vector_19 test-aes128 "90f42ec0f68385f2ffc5dfc03a654dce" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_19
  PROPERTIES PASS_REGULAR_EXPRESSION "7a20a53d460fc9ce0423a7a0764c6cf2")

add_test(NIST_KAT_ECBKeySbox128e_vector_20 test-aes128 "febd9a24d8b65c1c787d50a4ed3619a9" "00000000000000000000000000000000")
set_tests_properties (NIST_KAT_ECBKeySbox128e_vector_20
  PROPERTIES PASS_REGULAR_EXPRESSION "f4a70d8af877f9b02b4c40df57d45b17")

########################################
# autopath.at
########################################

add_test (autopath_basic1 ovs-ofctl.exe parse-flow "actions=autopath(1, NXM_NX_REG0[])")
set_tests_properties (autopath_basic1
  PROPERTIES PASS_REGULAR_EXPRESSION "usable protocols: any\nchosen protocol: OpenFlow10-table_id\nOFPT_FLOW_MOD .xid=0x1.: ADD actions=autopath.1,NXM_NX_REG0..")

add_test (autopath_basic2 ovs-ofctl parse-flow "actions=autopath(2, NXM_NX_REG0[2..30]")
set_tests_properties (autopath_basic2
  PROPERTIES PASS_REGULAR_EXPRESSION "usable protocols: any\nchosen protocol: OpenFlow10-table_id\nOFPT_FLOW_MOD .xid=0x1.: ADD actions=autopath.2,NXM_NX_REG0.2..30.")
#TODO
#], [stderr])
#AT_CHECK([[sed 's/^[^|]*|[^|]*|//' stderr]], [0], [dnl
#autopath|WARN|The autopath action is deprecated and may be removed in February 2013.  Please email dev@openvswitch.org with concerns.

add_test (autopath_action_missing_argument run_test.cmd ovs-ofctl parse-flow "actions=autopath")
set_tests_properties (autopath_action_missing_argument
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl: : not enough arguments to autopath action")

add_test (autopath_action_bad_port run_test.cmd ovs-ofctl parse-flow "actions=autopath(bad, NXM_NX_REG0[]")
set_tests_properties (autopath_action_bad_port
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl: bad, NXM_NX_REG0..: bad port number")

add_test (autopath_action_destination_too_narrow run_test.cmd ovs-ofctl parse-flow "actions=autopath(1,NXM_NX_REG0[0..7])")
set_tests_properties (autopath_action_destination_too_narrow
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl: 1,NXM_NX_REG0.0..7.: 8-bit destination field has 256 possible values, less than required 65536")

########################################
# bundle.at
########################################

# The test-bundle program prints a lot of output on stdout, but each of the
# tests below ignores it because it will vary a bit depending on endianness and
# floating point precision.  test-bundle will output an error message on
# stderr and return with exit code 1 if anything really goes wrong.  In each
# case, we list the (approximate) expected output in a comment to aid debugging
# if the test does fail.
add_test(hrw_bundle_link_selection test-bundle "symmetric_l4,60,hrw,ofport,NXM_NX_REG0[],slaves:1,2,3,4,5")
# 100000: disruption=1.00 (perfect=1.00) 1.00 0.00 0.00 0.00 0.00 0.00
# 110000: disruption=0.50 (perfect=0.50) 0.50 0.50 0.00 0.00 0.00 0.00
# 010000: disruption=0.50 (perfect=0.50) 0.00 1.00 0.00 0.00 0.00 0.00
# 011000: disruption=0.50 (perfect=0.50) 0.00 0.50 0.50 0.00 0.00 0.00
# 111000: disruption=0.33 (perfect=0.33) 0.33 0.33 0.34 0.00 0.00 0.00
# 101000: disruption=0.33 (perfect=0.33) 0.50 0.00 0.50 0.00 0.00 0.00
# 001000: disruption=0.50 (perfect=0.50) 0.00 0.00 1.00 0.00 0.00 0.00
# 001100: disruption=0.50 (perfect=0.50) 0.00 0.00 0.50 0.50 0.00 0.00
# 101100: disruption=0.33 (perfect=0.33) 0.33 0.00 0.34 0.33 0.00 0.00
# 111100: disruption=0.25 (perfect=0.25) 0.25 0.25 0.25 0.25 0.00 0.00
# 011100: disruption=0.25 (perfect=0.25) 0.00 0.33 0.33 0.33 0.00 0.00
# 010100: disruption=0.33 (perfect=0.33) 0.00 0.50 0.00 0.50 0.00 0.00
# 110100: disruption=0.33 (perfect=0.33) 0.33 0.33 0.00 0.34 0.00 0.00
# 100100: disruption=0.33 (perfect=0.33) 0.50 0.00 0.00 0.50 0.00 0.00
# 000100: disruption=0.50 (perfect=0.50) 0.00 0.00 0.00 1.00 0.00 0.00
# 000110: disruption=0.50 (perfect=0.50) 0.00 0.00 0.00 0.50 0.50 0.00
# 100110: disruption=0.33 (perfect=0.33) 0.33 0.00 0.00 0.33 0.33 0.00
# 110110: disruption=0.25 (perfect=0.25) 0.25 0.25 0.00 0.25 0.25 0.00
# 010110: disruption=0.25 (perfect=0.25) 0.00 0.34 0.00 0.33 0.33 0.00
# 011110: disruption=0.25 (perfect=0.25) 0.00 0.25 0.25 0.25 0.25 0.00
# 111110: disruption=0.20 (perfect=0.20) 0.20 0.20 0.20 0.20 0.20 0.00
# 101110: disruption=0.20 (perfect=0.20) 0.25 0.00 0.25 0.25 0.25 0.00
# 001110: disruption=0.25 (perfect=0.25) 0.00 0.00 0.34 0.33 0.33 0.00
# 001010: disruption=0.33 (perfect=0.33) 0.00 0.00 0.50 0.00 0.50 0.00
# 101010: disruption=0.33 (perfect=0.33) 0.33 0.00 0.34 0.00 0.33 0.00
# 111010: disruption=0.25 (perfect=0.25) 0.25 0.25 0.25 0.00 0.25 0.00
# 011010: disruption=0.25 (perfect=0.25) 0.00 0.33 0.34 0.00 0.33 0.00
# 010010: disruption=0.34 (perfect=0.33) 0.00 0.50 0.00 0.00 0.50 0.00
# 110010: disruption=0.33 (perfect=0.33) 0.33 0.33 0.00 0.00 0.33 0.00
# 100010: disruption=0.33 (perfect=0.33) 0.50 0.00 0.00 0.00 0.50 0.00
# 000010: disruption=0.50 (perfect=0.50) 0.00 0.00 0.00 0.00 1.00 0.00
# 000011: disruption=0.50 (perfect=0.50) 0.00 0.00 0.00 0.00 0.50 0.50
# 100011: disruption=0.33 (perfect=0.33) 0.33 0.00 0.00 0.00 0.33 0.33
# 110011: disruption=0.25 (perfect=0.25) 0.25 0.25 0.00 0.00 0.25 0.25
# 010011: disruption=0.25 (perfect=0.25) 0.00 0.33 0.00 0.00 0.33 0.33
# 011011: disruption=0.25 (perfect=0.25) 0.00 0.25 0.25 0.00 0.25 0.25
# 111011: disruption=0.20 (perfect=0.20) 0.20 0.20 0.20 0.00 0.20 0.20
# 101011: disruption=0.20 (perfect=0.20) 0.25 0.00 0.25 0.00 0.25 0.25
# 001011: disruption=0.25 (perfect=0.25) 0.00 0.00 0.34 0.00 0.33 0.33
# 001111: disruption=0.25 (perfect=0.25) 0.00 0.00 0.25 0.25 0.25 0.25
# 101111: disruption=0.20 (perfect=0.20) 0.20 0.00 0.20 0.20 0.20 0.20
# 111111: disruption=0.17 (perfect=0.17) 0.17 0.17 0.17 0.17 0.17 0.17
# 011111: disruption=0.17 (perfect=0.17) 0.00 0.20 0.20 0.20 0.20 0.20
# 010111: disruption=0.20 (perfect=0.20) 0.00 0.25 0.00 0.25 0.25 0.25
# 110111: disruption=0.20 (perfect=0.20) 0.20 0.20 0.00 0.20 0.20 0.20
# 100111: disruption=0.20 (perfect=0.20) 0.25 0.00 0.00 0.25 0.25 0.25
# 000111: disruption=0.25 (perfect=0.25) 0.00 0.00 0.00 0.33 0.33 0.33
# 000101: disruption=0.33 (perfect=0.33) 0.00 0.00 0.00 0.50 0.00 0.50
# 100101: disruption=0.33 (perfect=0.33) 0.33 0.00 0.00 0.33 0.00 0.33
# 110101: disruption=0.25 (perfect=0.25) 0.25 0.25 0.00 0.25 0.00 0.25
# 010101: disruption=0.25 (perfect=0.25) 0.00 0.33 0.00 0.33 0.00 0.33
# 011101: disruption=0.25 (perfect=0.25) 0.00 0.25 0.25 0.25 0.00 0.25
# 111101: disruption=0.20 (perfect=0.20) 0.20 0.20 0.20 0.20 0.00 0.20
# 101101: disruption=0.20 (perfect=0.20) 0.25 0.00 0.25 0.25 0.00 0.25
# 001101: disruption=0.25 (perfect=0.25) 0.00 0.00 0.33 0.33 0.00 0.33
# 001001: disruption=0.33 (perfect=0.33) 0.00 0.00 0.50 0.00 0.00 0.50
# 101001: disruption=0.33 (perfect=0.33) 0.33 0.00 0.33 0.00 0.00 0.33
# 111001: disruption=0.25 (perfect=0.25) 0.25 0.25 0.25 0.00 0.00 0.25
# 011001: disruption=0.25 (perfect=0.25) 0.00 0.33 0.34 0.00 0.00 0.33
# 010001: disruption=0.34 (perfect=0.33) 0.00 0.50 0.00 0.00 0.00 0.50
# 110001: disruption=0.33 (perfect=0.33) 0.33 0.33 0.00 0.00 0.00 0.34
# 100001: disruption=0.33 (perfect=0.33) 0.50 0.00 0.00 0.00 0.00 0.50
# 000001: disruption=0.50 (perfect=0.50) 0.00 0.00 0.00 0.00 0.00 1.00
# 000000: disruption=1.00 (perfect=1.00) 0.00 0.00 0.00 0.00 0.00 0.00
# 100000: disruption=1.00 (perfect=1.00) 1.00 0.00 0.00 0.00 0.00 0.00

add_test (active_backup_bundle_link_selection test-bundle "symmetric_l4,60,active_backup,ofport,NXM_NX_REG0[],slaves:1,2,3,4,5,6")
set_tests_properties (active_backup_bundle_link_selection
  PROPERTIES PASS_REGULAR_EXPRESSION "100000: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n110000: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n010000: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n011000: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n111000: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n101000: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n001000: disruption=1.00 .perfect=1.00. 0.00 0.00 1.00 0.00 0.00 0.00\n001100: disruption=0.00 .perfect=0.00. 0.00 0.00 1.00 0.00 0.00 0.00\n101100: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n111100: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n011100: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n010100: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n110100: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n100100: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n000100: disruption=1.00 .perfect=1.00. 0.00 0.00 0.00 1.00 0.00 0.00\n000110: disruption=0.00 .perfect=0.00. 0.00 0.00 0.00 1.00 0.00 0.00\n100110: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n110110: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n010110: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n011110: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n111110: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n101110: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n001110: disruption=1.00 .perfect=1.00. 0.00 0.00 1.00 0.00 0.00 0.00\n001010: disruption=0.00 .perfect=0.00. 0.00 0.00 1.00 0.00 0.00 0.00\n101010: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n111010: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n011010: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n010010: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n110010: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n100010: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n000010: disruption=1.00 .perfect=1.00. 0.00 0.00 0.00 0.00 1.00 0.00\n000011: disruption=0.00 .perfect=0.00. 0.00 0.00 0.00 0.00 1.00 0.00\n100011: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n110011: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n010011: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n011011: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n111011: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n101011: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n001011: disruption=1.00 .perfect=1.00. 0.00 0.00 1.00 0.00 0.00 0.00\n001111: disruption=0.00 .perfect=0.00. 0.00 0.00 1.00 0.00 0.00 0.00\n101111: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n111111: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n011111: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n010111: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n110111: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n100111: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n000111: disruption=1.00 .perfect=1.00. 0.00 0.00 0.00 1.00 0.00 0.00\n000101: disruption=0.00 .perfect=0.00. 0.00 0.00 0.00 1.00 0.00 0.00\n100101: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n110101: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n010101: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n011101: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n111101: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n101101: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n001101: disruption=1.00 .perfect=1.00. 0.00 0.00 1.00 0.00 0.00 0.00\n001001: disruption=0.00 .perfect=0.00. 0.00 0.00 1.00 0.00 0.00 0.00\n101001: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n111001: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n011001: disruption=1.00 .perfect=1.00. 0.00 1.00 0.00 0.00 0.00 0.00\n010001: disruption=0.00 .perfect=0.00. 0.00 1.00 0.00 0.00 0.00 0.00\n110001: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00\n100001: disruption=0.00 .perfect=0.00. 1.00 0.00 0.00 0.00 0.00 0.00\n000001: disruption=1.00 .perfect=1.00. 0.00 0.00 0.00 0.00 0.00 1.00\n000000: disruption=1.00 .perfect=1.00. 0.00 0.00 0.00 0.00 0.00 0.00\n100000: disruption=1.00 .perfect=1.00. 1.00 0.00 0.00 0.00 0.00 0.00")

add_test (hrw_bundle_single_link_selection test-bundle "symmetric_l4,60,hrw,ofport,NXM_NX_REG0[],slaves:1")
# 1: disruption=1.00 (perfect=1.00) 1.00
# 0: disruption=1.00 (perfect=1.00) 0.00
# 1: disruption=1.00 (perfect=1.00) 1.00

add_test (hrw_bundle_no_link_selection test-bundle "symmetric_l4,60,hrw,ofport,NXM_NX_REG0[],slaves:")
#: disruption=0.00 (perfect=0.00)
#: disruption=0.00 (perfect=0.00)

add_test (bundle_action_missing_argument ovs-ofctl.exe parse-flow actions=bundle)
set_tests_properties (bundle_action_missing_argument
 PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl.exe: : not enough arguments to bundle action")

add_test (bundle_action_bad_fields ovs-ofctl.exe parse-flow "actions=bundle(xyzzy,60,hrw,ofport,slaves:1,2)")
set_tests_properties (bundle_action_bad_fields
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl.exe: xyzzy,60,hrw,ofport,slaves:1,2: unknown fields .xyzzy.")

add_test (bundle_action_bad_algorithm ovs-ofctl.exe parse-flow "actions=bundle(symmetric_l4,60,fubar,ofport,slaves:1,2)")
set_tests_properties (bundle_action_bad_algorithm
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl.exe: symmetric_l4,60,fubar,ofport,slaves:1,2: unknown algorithm .fubar.")

add_test (bundle_action_bad_slave_type ovs-ofctl.exe parse-flow "actions=bundle(symmetric_l4,60,hrw,robot,slaves:1,2)")
set_tests_properties (bundle_action_bad_slave_type
  PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl.exe: symmetric_l4,60,hrw,robot,slaves:1,2: unknown slave_type .robot.")

add_test (bundle_action_bad_slave_delimiter ovs-ofctl.exe parse-flow "actions=bundle(symmetric_l4,60,hrw,ofport,robot:1,2)")
set_tests_properties (bundle_action_bad_slave_delimiter
    PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl.exe: symmetric_l4,60,hrw,ofport,robot:1,2: missing slave delimiter, expected .slaves. got .robot.")

########################################
# check-structs.at
########################################

# TODO

########################################
# classifier.at
########################################

add_test (flow_classifier_empty test-classifier empty)
add_test (flow_classifier_destroy-null test-classifier destroy-null)
add_test (flow_classifier_single-rule test-classifier single-rule)
add_test (flow_classifier_rule-replacement test-classifier rule-replacement)
add_test (flow_classifier_many-rules-in-one-list test-classifier many-rules-in-one-list)
add_test (flow_classifier_many-rules-in-one-table test-classifier many-rules-in-one-table)
add_test (flow_classifier_many-rules-in-two-tables test-classifier many-rules-in-two-tables)
add_test (flow_classifier_many-rules-in-five-tables test-classifier many-rules-in-five-tables)

add_test (miniflow_miniflow test-classifier miniflow)
add_test (miniflow_minimask_has_extra test-classifier minimask_has_extra)
add_test (miniflow_minimask_combine test-classifier minimask_combine)

########################################
# daemon-py.at
########################################

# TODO

########################################
# daemon.at
########################################

# TODO

########################################
# file_name.at
########################################

# These are the test cases given in POSIX for dirname() and basename().

add_test (test-file_name1 test-file_name /usr/lib)
set_tests_properties (test-file_name1
    PROPERTIES PASS_REGULAR_EXPRESSION "/usr\nlib\n")

add_test (test-file_name2 test-file_name /usr)
set_tests_properties (test-file_name2
    PROPERTIES PASS_REGULAR_EXPRESSION "/\nusr\n")

add_test (test-file_name3 test-file_name usr)
set_tests_properties (test-file_name3
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\nusr\n")

add_test (test-file_name4 test-file_name /)
set_tests_properties (test-file_name4
    PROPERTIES PASS_REGULAR_EXPRESSION "/\n/\n")

add_test (test-file_name5 test-file_name .)
set_tests_properties (test-file_name5
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\n\\.\n")

add_test (test-file_name6 test-file_name ..)
set_tests_properties (test-file_name6
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\n\\.\\.\n")

add_test (test-file_name7 test-file_name //)
set_tests_properties (test-file_name7
    PROPERTIES PASS_REGULAR_EXPRESSION "//\n//\n")

add_test (test-file_name8 test-file_name //foo)
set_tests_properties (test-file_name8
    PROPERTIES PASS_REGULAR_EXPRESSION "//\nfoo\n")

add_test (test-file_name9 test-file_name "")
set_tests_properties (test-file_name9
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\n\\.\n")

# Additional test cases.

add_test (test-file_name10 test-file_name dir/file)
set_tests_properties (test-file_name10
    PROPERTIES PASS_REGULAR_EXPRESSION "dir\nfile\n")

add_test (test-file_name11 test-file_name dir/file/)
set_tests_properties (test-file_name11
    PROPERTIES PASS_REGULAR_EXPRESSION "dir\nfile\n")

add_test (test-file_name12 test-file_name dir/file//)
set_tests_properties (test-file_name12
    PROPERTIES PASS_REGULAR_EXPRESSION "dir\nfile\n")

add_test (test-file_name13 test-file_name ///foo)
set_tests_properties (test-file_name13
    PROPERTIES PASS_REGULAR_EXPRESSION "/\nfoo\n")

# TODO rest of tests

########################################
# heap.at
########################################

add_test (heap-libray_insert-delete-same-order test-heap insert-delete-same-order)
add_test (heap-libray_insert-delete-reverse-order test-heap insert-delete-reverse-order)
add_test (heap-libray_insert-delete-every-order test-heap insert-delete-every-order)
add_test (heap-libray_insert-delete-same-order-with-dups test-heap insert-delete-same-order-with-dups)
add_test (heap-libray_raw-insert test-heap raw-insert)
add_test (heap-libray_raw-delete test-heap raw-delete)

######################################## 
# interface-reconfigure.at 
######################################## 
 
# TODO 

######################################## 
# json.at 
######################################## 
 
# TODO 

######################################## 
# jsonrpc-py.at 
######################################## 
 
# TODO 

######################################## 
# jsonrpc.at 
######################################## 
 
# TODO 

######################################## 
# lacp.at 
######################################## 
 
# TODO 

######################################## 
# learn.at 
######################################## 

file (WRITE flows.txt "actions=learn()\nactions=learn(NXM_OF_VLAN_TCI[0..11], NXM_OF_ETH_DST[]=NXM_OF_ETH_SRC[], output:NXM_OF_IN_PORT[], load:10->NXM_NX_REG0[5..10])\nactions=learn(table=1,idle_timeout=10, hard_timeout=20, fin_idle_timeout=5, fin_hard_timeout=10, priority=10, cookie=0xfedcba9876543210, in_port=99,NXM_OF_ETH_DST[]=NXM_OF_ETH_SRC[],load:NXM_OF_IN_PORT[]->NXM_NX_REG1[16..31])\n")
add_test (learning_action-parsing_and_formatting ovs-ofctl parse-flows flows.txt)
set_tests_properties (learning_action-parsing_and_formatting
    PROPERTIES PASS_REGULAR_EXPRESSION "usable protocols: any\nchosen protocol: OpenFlow10-table_id\nOFPT_FLOW_MOD \\(xid=0x1\\): ADD actions=learn\\(table=1\\)\nOFPT_FLOW_MOD \\(xid=0x2\\): ADD actions=learn\\(table=1,NXM_OF_VLAN_TCI\\[0\\.\\.11\\],NXM_OF_ETH_DST\\[\\]=NXM_OF_ETH_SRC\\[\\],output:NXM_OF_IN_PORT\\[\\],load:0xa->NXM_NX_REG0\\[5\\.\\.10\\]\\)\nOFPT_FLOW_MOD \\(xid=0x3\\): ADD actions=learn\\(table=1,idle_timeout=10,hard_timeout=20,fin_idle_timeout=5,fin_hard_timeout=10,priority=10,cookie=0xfedcba9876543210,in_port=99,NXM_OF_ETH_DST\\[\\]=NXM_OF_ETH_SRC\\[\\],load:NXM_OF_IN_PORT\\[\\]->NXM_NX_REG1\\[16\\.\\.31\\]\\)\n")
 
file (WRITE flows2.txt "# These are the examples from nicira-ext.h.\nactions=learn(in_port=99,NXM_OF_ETH_DST[]=NXM_OF_ETH_SRC[], load:NXM_OF_IN_PORT[]->NXM_NX_REG1[16..31])\nactions=learn(NXM_OF_VLAN_TCI[0..11], NXM_OF_ETH_DST[]=NXM_OF_ETH_SRC[],output:NXM_OF_IN_PORT[])\ntable=0 actions=learn(table=1,hard_timeout=10, NXM_OF_VLAN_TCI[0..11],output:NXM_OF_IN_PORT[]), resubmit(,1)\ntable=1 priority=0 actions=flood\n")
add_test (learning_action-examples ovs-ofctl parse-flows flows2.txt)
set_tests_properties (learning_action-examples
    PROPERTIES PASS_REGULAR_EXPRESSION "usable protocols: OpenFlow10\\+table_id,NXM\\+table_id\nchosen protocol: OpenFlow10\\+table_id\nOFPT_FLOW_MOD \\(xid=0x1\\): ADD table:255 actions=learn\\(table=1,in_port=99,NXM_OF_ETH_DST\\[\\]=NXM_OF_ETH_SRC\\[\\],load:NXM_OF_IN_PORT\\[\\]->NXM_NX_REG1\\[16\\.\\.31\\]\\)\nOFPT_FLOW_MOD \\(xid=0x2\\): ADD table:255 actions=learn\\(table=1,NXM_OF_VLAN_TCI\\[0\\.\\.11\\],NXM_OF_ETH_DST\\[\\]=NXM_OF_ETH_SRC\\[\\],output:NXM_OF_IN_PORT\\[\\]\\)\nOFPT_FLOW_MOD \\(xid=0x3\\): ADD actions=learn\\(table=1,hard_timeout=10,NXM_OF_VLAN_TCI\\[0\\.\\.11\\],output:NXM_OF_IN_PORT\\[\\]\\),resubmit\\(,1\\)\nOFPT_FLOW_MOD \\(xid=0x4\\): ADD table:1 priority=0 actions=FLOOD\n")

file (WRITE flows3.txt "actions=learn(eth_type=0x800,load:5->NXM_OF_IP_DST[])\nip,actions=learn(load:NXM_OF_IP_DST[]->NXM_NX_REG1[])\nip,actions=learn(eth_type=0x800,OXM_OF_IPV4_DST[])\n")
add_test (learning_action-satisfied_prerequisites ovs-ofctl parse-flows flows3.txt)
set_tests_properties (learning_action-satisfied_prerequisites
    PROPERTIES PASS_REGULAR_EXPRESSION "usable protocols: any\nchosen protocol: OpenFlow10-table_id\nOFPT_FLOW_MOD \\(xid=0x1\\): ADD actions=learn\\(table=1,eth_type=0x800,load:0x5->NXM_OF_IP_DST\\[\\]\\)\nOFPT_FLOW_MOD \\(xid=0x2\\): ADD ip actions=learn\\(table=1,load:NXM_OF_IP_DST\\[\\]->NXM_NX_REG1\\[\\]\\)\nOFPT_FLOW_MOD \\(xid=0x3\\): ADD ip actions=learn\\(table=1,eth_type=0x800,NXM_OF_IP_DST\\[\\]\\)\n")

add_test (learning_action-invalid_prerequisites1 run_test.cmd ovs-ofctl parse-flow "actions=learn(load:5->NXM_OF_IP_DST[])")
set_tests_properties (learning_action-invalid_prerequisites1
    PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl: load:5->NXM_OF_IP_DST\\[\\]: cannot specify destination field ip_dst because prerequisites are not satisfied\n")

add_test (learning_action-invalid_prerequisites2 run_test.cmd ovs-ofctl parse-flow "actions=learn(load:NXM_OF_IP_DST[]->NXM_NX_REG1[])")
set_tests_properties (learning_action-invalid_prerequisites2
    PROPERTIES PASS_REGULAR_EXPRESSION "ovs-ofctl: load:NXM_OF_IP_DST\\[\\]->NXM_NX_REG1\\[\\]: cannot specify source field ip_dst because prerequisites are not satisfied\n")

# TODO rest of tests

######################################## 
# library.at 
######################################## 

add_test (test_flow_extractor test_flow_extractor.cmd)
set_tests_properties (test_flow_extractor
    PROPERTIES PASS_REGULAR_EXPRESSION "checked 247 packets, 0 errors\n")

add_test (test_TCP_IP_checksumming test-csum)
set_tests_properties (test_TCP_IP_checksumming
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\\.\\.\\.#\\.\\.\\.\\.#\\.\\.\\.\\.##\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.#\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.\\.#\n")

add_test (test_hash_functions test-hash)

add_test (test_hash_map test-hmap)
set_tests_properties (test_hash_map
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\\.\\.\\.\\.\\.\\.\\.\\.\n")

add_test (test_linked_lists test-list)
set_tests_properties (test_linked_lists
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\\.\n")

add_test (test_packet_library test-packets)

add_test (test_SHA-1 test-sha1)
set_tests_properties (test_SHA-1
    PROPERTIES PASS_REGULAR_EXPRESSION "\\.\\.\\.\\.\\.\\.\\.\\.\\.\n")

add_test (test_type_properties test-type-props)

add_test (test_strtok_r_bug_fix test-strtok_r)
set_tests_properties (test_strtok_r_bug_fix
    PROPERTIES PASS_REGULAR_EXPRESSION "NULL NULL\n")

add_test (test_byte_order_conversion test-byte-order)

add_test (test_random_number_generator test-random)
set_tests_properties (test_random_number_generator
    PROPERTIES PASS_REGULAR_EXPRESSION "average=7fa2014f\n\nbit      0     1\n  0  4946  5054\n  1  4939  5061\n  2  4947  5053\n  3  4935  5065\n  4  5004  4996\n  5  4998  5002\n  6  5062  4938\n  7  5009  4991\n  8  5001  4999\n  9  5022  4978\n 10  5006  4994\n 11  5039  4961\n 12  4940  5060\n 13  5048  4952\n 14  4930  5070\n 15  4973  5027\n 16  4954  5046\n 17  5043  4957\n 18  5020  4980\n 19  5104  4896\n 20  5051  4949\n 21  5003  4997\n 22  5110  4890\n 23  4950  5050\n 24  5016  4984\n 25  5019  4981\n 26  4948  5052\n 27  4995  5005\n 28  4995  5005\n 29  4969  5031\n 30  5109  4891\n 31  4984  5016\n\\(expected values are 5000\\)\n\nnibble   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15\n     0 640 589 610 613 588 632 650 613 582 646 627 640 612 650 637 671\n     1 626 642 663 620 630 609 617 602 615 638 614 644 641 597 598 644\n     2 667 611 617 613 609 629 642 651 604 641 594 659 651 610 617 585\n     3 621 662 594 605 618 644 616 613 613 616 611 608 614 660 653 652\n     4 641 668 621 664 619 624 625 642 624 629 607 566 599 639 618 614\n     5 666 629 620 621 581 615 598 620 630 651 671 622 628 603 657 588\n     6 620 640 621 606 603 644 628 633 620 597 653 591 637 658 634 615\n     7 636 645 679 593 598 609 612 612 623 626 638 669 603 629 606 622\n\\(expected values are 625\\)\n")

add_test (ctz_function test-util ctz)
add_test (popcount_function test-util popcount)
add_test (log_2_floor_function test-util log_2_floor)
add_test (bitwise_copy_function test-util bitwise_copy)
add_test (bitwise_zero_function test-util bitwise_zero)
add_test (bitwise_one_function test-util bitwise_one)
add_test (bitwise_is_all_zeros_function test-util bitwise_is_all_zeros)

add_test (test_unix_socket_short_pathname test-unix-socket x)

add_test (test_unix_socket_long_pathname test_unix_socket_long_pathname.cmd)

######################################## 
# lockfile.at 
######################################## 

add_test (test-lockfile1 test-lockfile lock_and_unlock)
set_tests_properties (test-lockfile1
    PROPERTIES PASS_REGULAR_EXPRESSION "lock_and_unlock: success \\(0 children\\)\n")

add_test (test-lockfile2 test-lockfile lock_and_unlock_twice)
set_tests_properties (test-lockfile2
    PROPERTIES PASS_REGULAR_EXPRESSION "lock_and_unlock_twice: success \\(0 children\\)\n")

add_test (test-lockfile3 test-lockfile lock_blocks_same_process)
set_tests_properties (test-lockfile3
    PROPERTIES PASS_REGULAR_EXPRESSION "failed to lock file: Resource deadlock avoided\n")

add_test (test-lockfile4 test-lockfile lock_blocks_same_process_twice)
set_tests_properties (test-lockfile4
    PROPERTIES PASS_REGULAR_EXPRESSION "failed to lock file: Resource deadlock avoided\n.*failed to lock file: Resource deadlock avoided\n")

add_test (test-lockfile5 test-lockfile lock_blocks_other_process)
set_tests_properties (test-lockfile5
    PROPERTIES PASS_REGULAR_EXPRESSION ": child does not inherit lock\n.*: cannot lock file because it is already locked by pid")

add_test (test-lockfile6 test-lockfile lock_twice_blocks_other_process)
set_tests_properties (test-lockfile6
    PROPERTIES PASS_REGULAR_EXPRESSION ": failed to lock file: Resource deadlock avoided\n.*: child does not inherit lock\n.*: cannot lock file because it is already locked by pid")

add_test (test-lockfile7 test-lockfile lock_and_unlock_allows_other_process)
set_tests_properties (test-lockfile7
	PROPERTIES WILL_FAIL true)

add_test (test-lockfile8 test-lockfile lock_multiple)
set_tests_properties (test-lockfile8
    PROPERTIES PASS_REGULAR_EXPRESSION ": failed to lock file: Resource deadlock avoided\n")

add_test (test-lockfile9 test-lockfile lock_symlink)
set_tests_properties (test-lockfile9
    PROPERTIES PASS_REGULAR_EXPRESSION ": failed to lock file: Resource deadlock avoided\n.*: failed to lock file: Resource deadlock avoided\n.*: failed to lock file: Resource deadlock avoided\n.*: failed to lock file: Resource deadlock avoided\n")

add_test (test-lockfile10 test-lockfile lock_symlink_to_dir)
set_tests_properties (test-lockfile10
    PROPERTIES PASS_REGULAR_EXPRESSION ": failed to lock file: Resource deadlock avoided\n")

######################################## 
# multipath.at 
######################################## 
 
# TODO 

######################################## 
# odp.at 
######################################## 
 
# TODO 

######################################## 
# ofp-actions.at 
######################################## 
 
# TODO 

######################################## 
# ofp-errors.at 
######################################## 
 
# TODO 

######################################## 
# ofp-print.at 
######################################## 
 
# TODO 

######################################## 
# ofproto-dpif.at 
######################################## 
 
# TODO 

######################################## 
# ofproto-macros.at 
######################################## 
 
# TODO 

######################################## 
# ofproto.at 
######################################## 
 
# TODO 

######################################## 
# ovs-monitor-ipsec.at 
######################################## 
 
# TODO 

######################################## 
# ovs-ofctl.at 
######################################## 
 
# TODO 

######################################## 
# ovs-vsctl.at 
######################################## 
 
# TODO 

######################################## 
# ovs-xapi-sync.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-column.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-condition.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-data.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-execution.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-idl.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-log.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-macros.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-monitor.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-mutation.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-query.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-row.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-schema.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-server.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-table.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-tool.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-transaction.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-trigger.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb-types.at 
######################################## 
 
# TODO 

######################################## 
# ovsdb.at 
######################################## 
 
# TODO 

######################################## 
# reconnect.at 
######################################## 
 
# TODO 

######################################## 
# stp.at 
######################################## 
 
# TODO 

######################################## 
# testsuite.at 
######################################## 
 
# TODO 

######################################## 
# timeval.at 
######################################## 
 
# TODO 

######################################## 
# unixctl-py.at 
######################################## 
 
# TODO 

######################################## 
# uuid.at 
######################################## 
 
# TODO 

######################################## 
# vconn.at 
######################################## 
 
# TODO 

######################################## 
# vlog.at 
######################################## 
 
# TODO 

