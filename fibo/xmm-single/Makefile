# #########################################################
# This makefile builds the fibo/xmm program
#   
# Copyright (c) 2011 Parallax, Inc.
# All rights MIT licensed
# #########################################################

# Path to shared demo library
PROPLIB = ../../common

#
# name and objects for this program
#

NAME = fibo
OBJS = fibo.o

# memory model
MODEL ?= xmm-single

# flags to build with
CFLAGS = -Os

# default build target
all: $(NAME).elf

#
# include the fragment that defines most of everything
#
include $(PROPLIB)/common.mk
