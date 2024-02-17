GCC ?= gcc # Use default gcc compiler.

CFLAGS = -std=gnu99 -m64 -Wall -Wextra -pedantic -g


# Example usage:
# $(call run,gcc -o cat.o -c,"COMPILE", cat.c)
# This will compile source.c to output.o. 
# If V=1, gcc -o cat.o -c cat.c
V = 0	
ifeq ($(V),1)
run = $(1) $(3)
else
run = @$(if $(2),/bin/echo " " $(2) $(3) &&,) $(1) $(3)
endif

always:
	@:

.PHONY: always