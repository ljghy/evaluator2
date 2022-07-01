add_executable(eval)

set_property(TARGET eval PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin>)

if (MSVC)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT eval)
endif (MSVC)

target_sources(eval
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(eval
PRIVATE
    evaluator
)