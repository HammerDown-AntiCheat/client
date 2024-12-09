project(lazy_importer)
add_library(lazy_importer INTERFACE)
target_include_directories(lazy_importer INTERFACE lazy_importer/include)


project(xorstr)
add_library(xorstr INTERFACE)
target_include_directories(xorstr INTERFACE xorstr/include)
target_compile_definitions(xorstr INTERFACE LAZY_IMPORTER_HARDENED_MODULE_CHECKS)

add_subdirectory(code-virtualizer-sdk)