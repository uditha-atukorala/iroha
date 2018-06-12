# directory contains benchmarks

# default benchmark path is build/benchmark_bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark_bin)


add_executable(benchmark_db
    benchmark_db.cpp
    )
target_link_libraries(benchmark_db
    benchmark
    pqxx
    soci_core
    soci_postgresql
    iodbc
    )

add_executable(benchmark_example
    benchmark_example.cpp
    )
target_link_libraries(benchmark_example
    benchmark
    shared_model_stateless_validation
    )
