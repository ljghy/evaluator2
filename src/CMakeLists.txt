add_library(evaluator STATIC)

target_sources(evaluator
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Tokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AST.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InternalFunc.cpp
)

target_include_directories(evaluator
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)