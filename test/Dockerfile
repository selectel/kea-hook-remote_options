FROM ubuntu:20.04 AS base
ENV DEBIAN_FRONTEND noninteractive
ENV TZ=Europe/Moscow

RUN \
  ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
  apt-get -y update && \
  apt-get -y install \
    curl \
    gnupg \
    apt-utils \
    apt-transport-https && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
RUN \
  curl -1sLf 'https://dl.cloudsmith.io/public/isc/kea-2-0/gpg.8029D4AFA58CBB5E.key' | apt-key add - ;\
  curl -1sLf 'https://dl.cloudsmith.io/public/isc/kea-2-0/config.deb.txt?distro=ubuntu&codename=focal' > /etc/apt/sources.list.d/isc-kea-2-0.list


FROM base AS build
WORKDIR /build
RUN \
  apt-get -y update && \
  apt-get -y install \
    g++ \
    make \
    libboost-dev \
    libboost-system-dev \
    libcurl4-openssl-dev \
    isc-kea-dev \
    isc-kea-common
COPY src /build
RUN make

FROM base
LABEL description="Image to build kea-hook-remote-opts hook library"
RUN \
  ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
  apt-get -y update && \
  apt-get -y install \
    libcurl4 \
    libboost-system1.71.0 \
    isc-kea-common \
    isc-kea-dhcp4-server && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
COPY --from=build /kea-hook-remote-opts.so /usr/lib/x86_64-linux-gnu/kea/hooks/libremote_opts.so
COPY test/kea-dhcp4_test.conf /etc/kea/kea-dhcp4.conf

ENTRYPOINT [ "kea-dhcp4" ]
CMD [ "-c", "/etc/kea/kea-dhcp4.conf" ]
