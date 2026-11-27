#!/usr/bin/env bash
# 이미지에 설치된 도구와 버전을 출력한다. 하나라도 없으면 비정상 종료한다.
status=0
show() {
    local name="$1"; shift
    local out
    if out="$("$@" 2>&1 | head -n 1)"; then
        printf '%-14s %s\n' "$name" "$out"
    else
        printf '%-14s 없음\n' "$name"; status=1
    fi
}
show clang        clang --version
show clang++      clang++ --version
show clang-format clang-format --version
show llvm-cov     llvm-cov --version
show llvm-profdata llvm-profdata --version
show cmake        cmake --version
show ninja        ninja --version
show cppcheck     cppcheck --version
show lizard       lizard --version
show verreport    verreport --version
show cloc         cloc --version
show doxygen      doxygen --version
show pict         sh -c 'pict 2>&1 | head -n 1'
show gtest        sh -c 'dpkg-query -W -f="\${Package} \${Version}" libgtest-dev'
show gmock        sh -c 'dpkg-query -W -f="\${Package} \${Version}" libgmock-dev'
exit "$status"
