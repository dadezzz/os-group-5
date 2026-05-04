SOURCES  = $(shell find src/ -name '*.c')
OBJECTS  = $(patsubst src/%.c,build/objects/%.o,$(SOURCES))
DEPS     = $(patsubst %.o,%.d,$(OBJECTS))
COMMANDS = $(patsubst %.o,%.json,$(OBJECTS))

# Generate compile commands (for IDE autocomplete) only if Clang is used. GCC
# doesn't support it.
ifeq ($(CC),clang)
all: build build/compile_commands.json
CPPFLAGS += -MJ $(@:%.o=%.json)
else
all: build
endif

# Generate .d files to recompile when header files change.
CPPFLAGS += -MMD -MP
# Set C standard to 17 and allow some stricter diagnostics.
CFLAGS   += -std=c17 -pedantic-errors -Werror -Wall -Wcast-qual -Wconversion -Wextra -Wmissing-prototypes -Wnull-dereference -Wshadow
# For now we don't need special linker flags.
LDFLAGS  +=

# Include the header files generated with -MMD in $CPPFLAGS.
-include $(DEPS)

# Main executable.
build: build/restaurant

# Link the restaurant executable.
build/restaurant: $(OBJECTS) build/link_flags.txt
	@echo "LD $@"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@

build/objects/%.o: src/%.c build/compile_flags.txt
	@echo "CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Takes build/objects/%.json fragments and concatenates them into a json array.
# The first sed substitution adds [\n at the start of the file and the second
# removes the trailing comma and adds \n] at the end.
build/compile_commands.json: $(COMMANDS)
	@echo "created $@"
	@sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(COMMANDS) > $@

# Detect when LDFLAGS change and write them to a file, so that it can be used as
# a target dependency to trigger re-linkage.
#
# echo "__empty__" is used to avoid rebuilding when the flags are empty.
NEW_LINK_FLAGS = $(LDFLAGS)
OLD_LINK_FLAGS = $(shell cat build/link_flags.txt || echo "__empty__")
build/link_flags.txt: ALWAYS
	@mkdir -p $(dir $@)
	@if [ "$(NEW_LINK_FLAGS)" != "$(OLD_LINK_FLAGS)" ] ; then \
		echo "link flags changed"; \
		echo "$(NEW_LINK_FLAGS)" > $@; \
	fi

# Detect when CPPFLAGS or CFLAGS change and write them to a file so that it can
# be used as a target dependency to trigger re-compilation.
#
# echo "__empty__" is used to avoid rebuilding when the flags are empty.
NEW_COMPILE_FLAGS = $(CPPFLAGS) $(CFLAGS)
OLD_COMPILE_FLAGS = $(shell cat build/compile_flags.txt || echo "__empty__")
build/compile_flags.txt: ALWAYS
	@mkdir -p $(dir $@)
	@if [ "$(NEW_COMPILE_FLAGS)" != "$(OLD_COMPILE_FLAGS)" ] ; then \
		echo "compile flags changed"; \
		echo "$(NEW_COMPILE_FLAGS)" > $@; \
	fi

# Clean the build/ and submission/ dirs.
clean:
	@rm -rf build/ submission/

# Run ./bootstrap.sh
#
# TODO: read (optionally) args to pass from the ARGS variable.
run:
	./bootstrap.sh

# Copy files in a submission subdir with the layout that professors expect.
submission: Makefile src/ bootstrap.sh status.sh
	@rm -rf submission/
	@mkdir -p submission/code/
	@cp -vr Makefile bootstrap.sh status.sh src/ submission/code/
	@# cp report.pdf submission/

clang-format/fix:
	@clang-format -i $(SOURCES)
clang-format/check:
	@clang-format --dry-run --Werror $(SOURCES)
clang-tidy/check:
	@clang-tidy -p build/ --use-color --warnings-as-errors=* $(SOURCES)

# Used to force file-generating targets to always run.
ALWAYS:
.PHONY: clean run clang-format/fix clang-format/check clang-tidy/check ALWAYS
