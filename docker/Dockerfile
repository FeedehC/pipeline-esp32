FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG GH_RUNNER_VERSION="2.299.1"

ENV RUNNER_NAME=""
ENV RUNNER_WORK_DIRECTORY="_work"
ENV RUNNER_TOKEN=""
ENV RUNNER_REPOSITORY_URL=""
ENV RUNNER_LABELS=""
ENV RUNNER_ALLOW_RUNASROOT=true
ENV GITHUB_ACCESS_TOKEN=""
ENV AGENT_TOOLSDIRECTORY=/opt/hostedtoolcache

RUN DEBIAN_FRONTEND=noninteractive apt update && \
    apt install -y \
        curl \
        tar \
        apt-transport-https \
        ca-certificates \
        software-properties-common \
        sudo \
        jq \
        iputils-ping \
        build-essential \
        zlib1g-dev \
        gettext \
        liblttng-ust0 \
        libcurl4-openssl-dev && \
    rm -rf /var/lib/apt/lists/* && \
    apt clean

WORKDIR /home/runner

RUN mkdir actions-runner && cd actions-runner

RUN curl -o actions-runner-linux-x64-${GH_RUNNER_VERSION}.tar.gz -L https://github.com/actions/runner/releases/download/v${GH_RUNNER_VERSION}/actions-runner-linux-x64-${GH_RUNNER_VERSION}.tar.gz

RUN tar xzf ./actions-runner-linux-x64-${GH_RUNNER_VERSION}.tar.gz

COPY ./entrypoint.sh ./entrypoint.sh

RUN chmod +x ./config.sh ./run.sh ./entrypoint.sh

ENTRYPOINT ["./entrypoint.sh"]