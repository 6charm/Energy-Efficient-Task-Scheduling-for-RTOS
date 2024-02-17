TESTS = cat blockcat randomblockcat# build these executables
STDIOTESTS = $(patsubst %, stdio-%, $(TESTS))
SLOWTESTS = $(patsubst %, slow-%, $(TESTS))

# Optimization level
O ?= 0

all: $(TESTS) $(STDIOTESTS)
slow: $(SLOWTESTS)
files: $(FILES)

-include build/rules.mk
# @echo "Generating 1meg, 5meg, 20meg"

# Ref: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
# $@: Name of current target, $<: target first pre-req (cat.c)
# 
# *.c -> *.o iff *.c, aIO.h in same directory.
# All files depend on aIO.h, even profiler.
%.o: %.c aIO.h
	$(call run, $(CC) $(CFLAGS) -O$(O) -o $@ -c, COMPILE, $<)

# Build each target in TESTS by linking
# aIO.o, profile.o and test.o
# %: Associate % with each target in TESTS <static pattern rule>
# ref: https://www.gnu.org/software/make/manual/html_node/Static-Usage.html
$(TESTS): %: aIO.o profile.o %.o
	$(call run, $(CC) $(CFLAGS) -o $@ $^, LINK $@)

# slow-cat, slow-blockcat, slow-randomcat
$(SLOWTESTS): slow-%: slow-aIO.o profile.o %.o
	$(call run,$(CC) $(CFLAGS) -o $@ $^)

$(STDIOTESTS): stdio-%: stdio-aIO.o profile.o %.o
	$(call run,$(CC) $(CFLAGS) -o $@ $^)

$(FILES): 
	python3 generate-text-files.py

clean:
	$(call run,rm -f $(TESTS) $(SLOWTESTS) $(STDIOTESTS) *.o core *.core,CLEAN)
	$(call run,rm -rf files)

.PHONY: files, clean