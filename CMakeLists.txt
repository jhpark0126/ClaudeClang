cmake_minimum_required(VERSION 3.25)
project(ClaudeClang LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 개발 빌드와 CI 빌드가 같은 옵션을 쓴다.
option(ENABLE_COVERAGE "LLVM 소스 기반 커버리지(MC/DC 포함)를 켠다" OFF)

if(ENABLE_COVERAGE)
  # -O0: 조건/분기가 최적화로 합쳐지지 않게 한다. MC/DC는 clang 18 이상 필요.
  add_compile_options(-O0 -g -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc)
  add_link_options(-fprofile-instr-generate)
endif()

find_package(GTest REQUIRED)
enable_testing()

# 제품 코드: src/ 아래 모든 .c/.cpp 를 정적 라이브러리로 묶는다.
file(GLOB_RECURSE PRODUCT_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_SOURCE_DIR}/src/*.c ${CMAKE_SOURCE_DIR}/src/*.cpp)
if(PRODUCT_SOURCES)
  add_library(product STATIC ${PRODUCT_SOURCES})
  target_include_directories(product PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
  # 제품 코드는 경고 없이 컴파일되어야 한다 (코드 체크리스트 COD-11)
  target_compile_options(product PRIVATE -Wall -Wextra -Werror)
endif()

# 시험 묶음은 서로 섞지 않는다. 각각 별도 실행 파일 = 별도 결과 = 별도 커버리지.
#   test/tdd         : 구현 중 TDD로 만든 테스트
#   test/unit        : 단위 테스트 (MC/DC 100%)
#   test/integration : 통합 테스트 (함수 커버리지 100%)
function(add_test_suite name dir)
  file(GLOB_RECURSE suite_sources CONFIGURE_DEPENDS
       ${CMAKE_SOURCE_DIR}/${dir}/*.c ${CMAKE_SOURCE_DIR}/${dir}/*.cpp)
  if(suite_sources)
    add_executable(${name} ${suite_sources})
    target_link_libraries(${name} PRIVATE GTest::gmock GTest::gmock_main GTest::gtest)
    if(TARGET product)
      target_link_libraries(${name} PRIVATE product)
    endif()
    add_test(NAME ${name} COMMAND ${name})
  endif()
endfunction()

add_test_suite(tdd_tests test/tdd)
add_test_suite(unit_tests test/unit)
add_test_suite(integration_tests test/integration)
