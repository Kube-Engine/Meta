project(KubeMetaTests)

get_filename_component(KubeMetaTestsDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeMetaTestsSources
    ${KubeMetaTestsDir}/tests_TemplateDecomposer.cpp
    ${KubeMetaTestsDir}/tests_Constructor.cpp
    ${KubeMetaTestsDir}/tests_Converter.cpp
    ${KubeMetaTestsDir}/tests_Data.cpp
    ${KubeMetaTestsDir}/tests_Type.cpp
    ${KubeMetaTestsDir}/tests_Var.cpp
    ${KubeMetaTestsDir}/tests_Signal.cpp
    ${KubeMetaTestsDir}/tests_SlotTable.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${KubeMetaTestsSources})

add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${CMAKE_PROJECT_NAME})

target_link_libraries(${CMAKE_PROJECT_NAME}
PUBLIC
    KubeMeta
    GTest::GTest GTest::Main
)

if(KF_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
    target_link_options(${PROJECT_NAME} PUBLIC --coverage)
endif()