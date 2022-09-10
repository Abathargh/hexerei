FROM ubuntu:20.04

WORKDIR /app
ENV CC=gcc-9

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

RUN apt-get -y update && apt-get -y install gpg wget \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get -y update && apt-get -y install gcc-9 make cmake git \ 
    && git clone --recurse-submodules https://github.com/Abathargh/hexerei

WORKDIR /app/hexerei/build

RUN cmake .. -B .
ENTRYPOINT ["make", "test"]

