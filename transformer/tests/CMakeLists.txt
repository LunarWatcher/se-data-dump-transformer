add_executable(tests
    Runner.cpp

    src/CheckSchema.cpp
    src/TestUnicodeManagement.cpp
    src/FilterTests.cpp
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests sedd-src Catch2::Catch2)
