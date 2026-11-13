# ---------- build stage ----------
FROM postgres:18 AS build

# Dev build by default. For benchmarking, build with:
#   docker build --build-arg CFLAGS_OPT="-O2" --build-arg CASSERT="" -t team10-postgres:custom .
ARG CFLAGS_OPT="-O0"
ARG CASSERT="--enable-cassert"

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential bison flex perl pkg-config curl ca-certificates bzip2 \
      libreadline-dev zlib1g-dev libssl-dev libicu-dev libxml2-dev \
      libxslt1-dev uuid-dev liblz4-dev libzstd-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Download the same version as the base image
RUN PGVER=$(postgres --version | awk '{print $3}') \
 && curl -fsSL https://ftp.postgresql.org/pub/source/v${PGVER}/postgresql-${PGVER}.tar.bz2 \
    | tar xj --strip-components=1

# Compile the stock source once
RUN ./configure \
      --prefix=/usr/lib/postgresql/18 \
      --enable-depend --enable-debug ${CASSERT} \
      --with-openssl --with-icu --with-libxml --with-libxslt \
      --with-uuid=e2fs --with-lz4 --with-zstd \
      CFLAGS="${CFLAGS_OPT} -g" \
 && make -j"$(nproc)"

# Overlay modified PostgreSQL source files
# Includes:
#   src/backend/executor/nodeHashjoin.c
#   src/backend/executor/nodeHash.c
#   src/include/nodes/execnodes.c
COPY src/ /overlay/

RUN cp -r /overlay/. /src/src/ \
 && find /overlay -type f | while read -r file; do \
      touch "/src/src/${file#/overlay/}"; \
    done \
 && make -j"$(nproc)" \
 && make install \
 && make -C contrib -j"$(nproc)" install

# ---------- runtime stage ----------
FROM postgres:18

COPY --from=build /usr/lib/postgresql/18 /usr/lib/postgresql/18