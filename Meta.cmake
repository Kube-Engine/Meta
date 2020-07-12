project(KubeMeta)

get_filename_component(KubeMetaDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeMetaSources
    ${KubeMetaDir}/Meta.hpp
    ${KubeMetaDir}/Base.hpp
    ${KubeMetaDir}/Base.ipp
    ${KubeMetaDir}/Constructor.hpp
    ${KubeMetaDir}/Constructor.ipp
    ${KubeMetaDir}/Converter.hpp
    ${KubeMetaDir}/Converter.ipp
    ${KubeMetaDir}/Data.hpp
    ${KubeMetaDir}/Data.ipp
    ${KubeMetaDir}/Factory.hpp
    ${KubeMetaDir}/Factory.ipp
    ${KubeMetaDir}/Forward.hpp
    ${KubeMetaDir}/Function.hpp
    ${KubeMetaDir}/Function.ipp
    ${KubeMetaDir}/Register.cpp
    ${KubeMetaDir}/Resolver.hpp
    ${KubeMetaDir}/Resolver.cpp
    ${KubeMetaDir}/Signal.hpp
    ${KubeMetaDir}/Signal.ipp
    ${KubeMetaDir}/Type.hpp
    ${KubeMetaDir}/Type.ipp
    ${KubeMetaDir}/Type.cpp
    ${KubeMetaDir}/Var.hpp
    ${KubeMetaDir}/Var.ipp
    ${KubeMetaDir}/Var.cpp
)

add_library(${PROJECT_NAME} ${KubeMetaSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    KubeCore
    ParallelHashmap
)

if(${KF_TESTS})
    include(${KubeMetaDir}/Tests/MetaTests.cmake)
endif()

if(${KF_BENCHMARKS})
    include(${KubeMetaDir}/Benchmarks/MetaBenchmarks.cmake)
endif()