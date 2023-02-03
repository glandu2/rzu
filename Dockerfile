# syntax=docker/dockerfile:1.3

FROM alpine:latest as builder

WORKDIR /
RUN apk add --no-cache build-base ninja cmake git openssl3-dev unixodbc-dev
RUN apk add --no-cache --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing sqliteodbc && \
    printf '\n\
[SQLite3]\n\
Description=SQLite ODBC Driver\n\
Driver=libsqlite3odbc.so\n\
Setup=libsqlite3odbc.so\n\
Threading=2\n\
' | odbcinst -i -d -r

COPY . /source

RUN cmake -S /source -B /build -GNinja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=TRUE && \
    cmake --build /build --config Release && \
    cmake --install /build --prefix /install/deps --component deps && \
    cmake --install /build --prefix /install/rzauctionmonitor --component rzauctionmonitor && \
    cmake --install /build --prefix /install/rzauth --component rzauth && \
    cmake --install /build --prefix /install/rzbenchauth --component rzbenchauth && \
    cmake --install /build --prefix /install/rzbenchlog --component rzbenchlog && \
    cmake --install /build --prefix /install/rzchatgateway --component rzchatgateway && \
    cmake --install /build --prefix /install/rzclientreconnect --component rzclientreconnect && \
    cmake --install /build --prefix /install/rzfilter --component rzfilter && \
    cmake --install /build --prefix /install/rzgame --component rzgame && \
    cmake --install /build --prefix /install/rzgamereconnect --component rzgamereconnect && \
    cmake --install /build --prefix /install/rzlog --component rzlog && \
    cmake --install /build --prefix /install/rzplayercount --component rzplayercount && \
    cmake --install /build --prefix /install/rzrequester --component rzrequester && \
    cmake --install /build --prefix /install/rztestgs --component rztestgs && \
    rm -rf /build

FROM alpine:latest as base
WORKDIR /root/
RUN apk add --no-cache libstdc++ libssl3 unixodbc psqlodbc
COPY --from=builder /install/deps/ /root/


FROM base
ARG program
ENV main_program=$program

COPY --from=builder /install/${program}/ /root/
ENTRYPOINT /root/$main_program /core.log.level:fatal /core.log.consolelevel:info
