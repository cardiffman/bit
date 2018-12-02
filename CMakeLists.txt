project(bit C CXX)

add_executable(bit bit.cpp jsoncpp.cpp bitwindow.cpp XCBWindow.cpp)

set_property(TARGET bit PROPERTY CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

pkg_check_modules(KS REQUIRED xcb-keysyms)

pkg_check_Modules(XCB REQUIRED xcb)

target_link_libraries(bit ${KS_LIBRARIES} ${XCB_LIBRARIES})
target_include_directories(bit PUBLIC ${KS_INCLUDE_DIRS} ${XCB_INCLUDE_DIRS})
target_compile_options(bit PUBLIC ${SDL2_CFLAGS_OTHER} ${XCB_CFLAGS_OTHER})

