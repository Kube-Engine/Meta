project(KubeMetaBenchmarks)

get_filename_component(KubeMetaBenchmarksDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeMetaBenchmarksSources
    ${KubeMetaBenchmarksDir}/Main.cpp
    ${KubeMetaBenchmarksDir}/bench_Signal.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${KubeMetaBenchmarksSources})

target_link_libraries(${CMAKE_PROJECT_NAME}
PUBLIC
    KubeMeta
    benchmark::benchmark
)