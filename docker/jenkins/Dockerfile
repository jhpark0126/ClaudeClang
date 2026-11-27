# Jenkins 안정화(LTS) 버전 + Docker CLI + 필요한 플러그인
# 파이프라인은 호스트의 Docker 데몬(소켓 마운트)을 써서 claudeclangbuild 이미지 안에서 빌드한다.
FROM jenkins/jenkins:lts-jdk21

USER root

# Docker CLI (데몬은 호스트 것을 쓴다)
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl gnupg jq \
    && install -m 0755 -d /etc/apt/keyrings \
    && curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian $(. /etc/os-release && echo "$VERSION_CODENAME") stable" \
        > /etc/apt/sources.list.d/docker.list \
    && apt-get update && apt-get install -y --no-install-recommends docker-ce-cli \
    && rm -rf /var/lib/apt/lists/*

COPY plugins.txt /usr/share/jenkins/ref/plugins.txt
RUN jenkins-plugin-cli --plugin-file /usr/share/jenkins/ref/plugins.txt \
    && jenkins-plugin-cli --list > /usr/share/jenkins/ref/plugins.installed.txt 2>/dev/null || true

COPY casc.yaml /usr/share/jenkins/ref/casc.yaml
