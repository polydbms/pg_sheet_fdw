# Docker Image for compiling and testing the SheetReader FDW
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive

# install prerequisites
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        gcc \
        git \
        libc-dev \
        make \
        cmake \
        ca-certificates \
        libssl-dev \
        curl \
        nano \
        gpg  \
        gnupg2  \
        software-properties-common  \
        apt-transport-https  \
        lsb-release  \
        gdb \
        g++ \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://www.postgresql.org/media/keys/ACCC4CF8.asc| gpg --dearmor -o /etc/apt/trusted.gpg.d/postgresql.gpg
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ `lsb_release -cs`-pgdg main" | tee  /etc/apt/sources.list.d/pgdg.list

RUN apt-get update &&  apt-get install -y --no-install-recommends \
                              postgresql-13 \
                              postgresql-server-dev-13

# Copy files and directories from the host machine to the container
RUN mkdir /pg_sheet_fdw
COPY pg_sheet_fdw.control /pg_sheet_fdw/
COPY pg_sheet_fdw--0.1.sql /pg_sheet_fdw/
COPY Makefile /pg_sheet_fdw/
COPY src /pg_sheet_fdw/src
COPY include /pg_sheet_fdw/include
COPY submodules /pg_sheet_fdw/submodules
COPY test /pg_sheet_fdw/test

# Run installation commands
RUN echo "Installing pg_sheet_fdw." \
    && cd /pg_sheet_fdw/ && make USE_PGXS=1 install

USER postgres

RUN    /etc/init.d/postgresql start &&\
    psql --command "ALTER USER postgres WITH PASSWORD '123456';" &&\
    createdb -O postgres db1

RUN echo "host all  all    0.0.0.0/0  md5" >> /etc/postgresql/13/main/pg_hba.conf

RUN echo "listen_addresses='*'" >> /etc/postgresql/13/main/postgresql.conf
EXPOSE 5432

VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]

CMD ["/usr/lib/postgresql/13/bin/postgres", "-D", "/var/lib/postgresql/13/main", "-c", "config_file=/etc/postgresql/13/main/postgresql.conf"]