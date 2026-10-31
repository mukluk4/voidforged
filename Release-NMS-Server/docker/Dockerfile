# Voidforged server image. Build context is Release-NMS-Server (see docker-compose.yml).
# BUILD_JOBS: parallel compiles. Each can use 1-2 GB of memory, so keep it low on a small machine.
FROM debian:bookworm AS build
RUN apt-get update && apt-get install -y --no-install-recommends build-essential cmake git ninja-build \
    pkg-config libtool make libsodium-dev libmariadb-dev libssl-dev zlib1g-dev uuid-dev libcurl4-openssl-dev \
    libyaml-cpp-dev libperl-dev perl liblua5.1-dev libluabind-dev libboost-dev
COPY . /src
ARG BUILD_JOBS=1
RUN cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS_RELEASE="-O2 -DNDEBUG" -DCMAKE_C_FLAGS_RELEASE="-O2 -DNDEBUG" \
        -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_TESTS=OFF -DEQEMU_BUILD_PCH=OFF \
    && ninja -C /build -j${BUILD_JOBS} world zone eqlaunch shared_memory ucs queryserv loginserver export_client_files import_client_files

FROM debian:bookworm-slim AS server
RUN apt-get update && apt-get install -y --no-install-recommends libsodium23 libmariadb3 libssl3 zlib1g libuuid1 \
    libcurl4 libyaml-cpp0.7 libperl5.36 perl liblua5.1-0 libjson-perl libio-stringy-perl libdbi-perl \
    libdbd-mysql-perl mariadb-client ca-certificates procps
COPY --from=build /build/bin/ /opt/vf/bin/
COPY utils/patches/ /opt/vf/patches/
COPY docker/start.sh /opt/vf/start.sh
# A Windows checkout gives these CRLF line endings, which sh can't run
RUN sed -i 's/\r$//' /opt/vf/start.sh
WORKDIR /data
ENTRYPOINT ["sh", "/opt/vf/start.sh"]
