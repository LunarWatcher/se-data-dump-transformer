FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && apt-get upgrade -y \
        && apt-get install -y libssl-dev cmake gcc g++ git

ARG SEDD_OUTPUT_TYPE=json
ENV SPDLOG_LEVEL=info

COPY transformer/src /app/src
COPY transformer/tests /app/tests
COPY transformer/CMakeLists.txt /app/CMakeLists.txt
COPY transformer/cmake /app/cmake
COPY transformer/dep_provider.cmake /app/dep_provider.cmake

RUN mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j $(nproc)


CMD ls -la ./downloads && ls -la ./out \
        && /app/build/bin/sedd-transformer -i /app/downloads -o /app/out -t ${SEDD_OUTPUT_TYPE} || sleep 460000
