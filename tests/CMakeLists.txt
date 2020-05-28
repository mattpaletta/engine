add_executable(test_engine test_engine.cpp)
target_link_libraries(test_engine engine Catch2::Catch2)

add_test(engine test_engine)
