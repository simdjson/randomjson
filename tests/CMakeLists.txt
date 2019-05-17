set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
include_directories("../include")
include_directories("dependencies/fastvalidate-utf-8/include")
include_directories("dependencies/simdjson/singleheader")
add_executable (tests tests.cpp)

macro(append var string)
  set(${var} "${${var}} ${string}")
endmacro(append)

# some compilers like clang do not automagically define __AVX2__ and __BMI2__ even when the hardware supports it
if(NOT MSVC)
   set (OPT_FLAGS "${OPT_FLAGS} -mavx2 -mbmi -mbmi2 -mpclmul")
else()
   set (OPT_FLAGS "${OPT_FLAGS} /arch:AVX2 /std:c++latest")
endif()

append(CMAKE_CXX_FLAGS ${OPT_FLAGS})