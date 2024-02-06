#! /bin/bash

# This script calls the Makefile for pgsql extension compiling


make USE_PGXS=1 $1
