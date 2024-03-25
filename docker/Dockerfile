# Docker Image for compiling and testing the SheetReader FDW
FROM postgresdb:latest

USER root

# Get compiler and debugger
RUN apt-get update && apt-get install -qy g++ gdb

# Copy files and directories from the host machine to the container
RUN mkdir /pg_sheet_fdw
COPY pg_sheet_fdw.control /pg_sheet_fdw/
COPY pg_sheet_fdw--0.1.sql /pg_sheet_fdw/
COPY Makefile /pg_sheet_fdw/
COPY src /pg_sheet_fdw/src
COPY include /pg_sheet_fdw/include
COPY submodules /pg_sheet_fdw/submodules

# Run installation commands
RUN echo "Installing pg_sheet_fdw." \
    && cd /pg_sheet_fdw/ && make USE_PGXS=1 install

USER postgres