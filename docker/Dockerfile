# 개발 빌드와 CI 빌드가 함께 쓰는 빌드 이미지 (이름: claudeclangbuild)
# 빌드: bash scripts/build-image.sh
FROM ubuntu:24.04

ARG LLVM_VERSION=22
ARG PICT_VERSION=v3.7.4
ARG VERREPORT_VERSION=v0.1.1
ARG DEBIAN_FRONTEND=noninteractive

ENV LANG=C.UTF-8 \
    CC=clang \
    CXX=clang++ \
    PATH=/opt/venv/bin:$PATH

# 기본 도구: 빌드, 정적 분석(Cppcheck), 테스트(GoogleTest/GMock), 주석률(CLOC), 문서(Doxygen)
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl gnupg git make g++ cmake ninja-build \
        python3 python3-venv \
        cppcheck cloc doxygen graphviz \
        libgtest-dev libgmock-dev \
        jq xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Clang/LLVM (apt.llvm.org): 컴파일러, MC/DC 커버리지(llvm-cov, llvm-profdata), 포맷터
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /usr/share/keyrings/llvm.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] http://apt.llvm.org/noble/ llvm-toolchain-noble-${LLVM_VERSION} main" \
        > /etc/apt/sources.list.d/llvm.list \
    && apt-get update && apt-get install -y --no-install-recommends \
        clang-${LLVM_VERSION} clang-format-${LLVM_VERSION} clang-tidy-${LLVM_VERSION} \
        llvm-${LLVM_VERSION} lld-${LLVM_VERSION} libclang-rt-${LLVM_VERSION}-dev \
    && for t in clang clang++ clang-format clang-tidy llvm-cov llvm-profdata llvm-config; do \
         ln -sf /usr/bin/${t}-${LLVM_VERSION} /usr/local/bin/${t}; \
       done \
    && rm -rf /var/lib/apt/lists/*

# Lizard(순환복잡도/함수 길이), lcov_cobertura(커버리지를 Jenkins가 읽는 형식으로 변환)
RUN python3 -m venv /opt/venv \
    && /opt/venv/bin/pip install --no-cache-dir lizard lcov_cobertura

# VERReport (검증 결과 대시보드: gtest·llvm-cov·Cppcheck·Lizard 결과를 HTML 한 묶음으로)
RUN /opt/venv/bin/pip install --no-cache-dir \
        "git+https://github.com/jhpark0126/VERReport.git@${VERREPORT_VERSION}"

# PICT (조합 테스트 생성) — 소스에서 빌드
RUN git clone --depth 1 --branch ${PICT_VERSION} https://github.com/microsoft/pict.git /tmp/pict \
    && make -C /tmp/pict -j"$(nproc)" \
    && install -m 0755 /tmp/pict/pict /usr/local/bin/pict \
    && rm -rf /tmp/pict

# 컨테이너가 임의 UID로 실행되어도(Jenkins) git이 저장소를 거부하지 않게 한다
RUN git config --system --add safe.directory '*'

COPY docker/tool-versions.sh /usr/local/bin/tool-versions
RUN chmod 0755 /usr/local/bin/tool-versions

WORKDIR /work
CMD ["bash"]
