SOURCES  = $(shell find src/ -name '*.c')
OBJECTS  = $(patsubst src/%.c,build/objects/%.o,$(SOURCES))
DEPS     = $(patsubst %.o,%.d,$(OBJECTS))
COMMANDS = $(patsubst %.o,%.json,$(OBJECTS))

all: build-debug

# Generate .d files to recompile when header files change.
CPPFLAGS += -MMD -MP
# Set C standard to 23 and enable some stricter diagnostics.
CFLAGS   += -std=gnu23 -pedantic -Wall -Wcast-qual -Wconversion -Wextra -Wmissing-prototypes -Wnull-dereference -Wshadow
# Add libm for ceil and pow functions.
LDFLAGS  += -pthread -lm

# Include the header files generated with -MMD in $CPPFLAGS.
-include $(DEPS)

# Add debug symbols and sanitizations to detect leaks and undefined behaviour.
build-debug: CFLAGS += -g -fsanitize=address -fsanitize=undefined
build-debug: LDFLAGS += -fsanitize=address -fsanitize=undefined
# build-debug: CFLAGS += -g -fsanitize=thread -fsanitize=undefined
# build-debug: LDFLAGS += -fsanitize=thread -fsanitize=undefined
build-debug: build/restaurant
# Generate compile commands (for IDE autocomplete) only if Clang is used. GCC
# doesn't support it.
ifeq ($(CC),clang)
build-debug: CPPFLAGS += -MJ $(@:%.o=%.json)
build-debug: build/compile_commands.json
endif

# Add optimization flags.
build-release: CFLAGS += -O3
build-release: LDFLAGS += -s
build-release: build/restaurant

# Link the restaurant executable.
build/restaurant: $(OBJECTS) build/link_flags.txt
	@echo "LD $@"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Create object and compile_commands files for each source file.
build/objects/%.o build/objects/%.json: src/%.c build/compile_flags.txt
	@echo "CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Takes build/objects/%.json fragments and concatenates them into a json array.
# The first sed substitution adds [\n at the start of the file and the second
# removes the trailing comma and adds \n] at the end.
build/compile_commands.json: $(COMMANDS)
	@echo "GEN $@"
	@sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(COMMANDS) > $@

# Detect when LDFLAGS change and write them to a file, so that it can be used as
# a target dependency to trigger re-linkage.
#
# echo "__empty__" is used to avoid rebuilding when the flags are empty.
NEW_LINK_FLAGS = $(LDFLAGS)
OLD_LINK_FLAGS = $(shell cat build/link_flags.txt 2>/dev/null || echo "__empty__")
build/link_flags.txt: ALWAYS
	@mkdir -p $(dir $@)
	@if [ "$(NEW_LINK_FLAGS)" != "$(OLD_LINK_FLAGS)" ] ; then \
		echo "GEN $@"; \
		echo "$(NEW_LINK_FLAGS)" > $@; \
	fi

# Detect when CPPFLAGS or CFLAGS change and write them to a file so that it can
# be used as a target dependency to trigger re-compilation.
#
# echo "__empty__" is used to avoid rebuilding when the flags are empty.
NEW_COMPILE_FLAGS = $(CPPFLAGS) $(CFLAGS)
OLD_COMPILE_FLAGS = $(shell cat build/compile_flags.txt 2>/dev/null || echo "__empty__")
build/compile_flags.txt: ALWAYS
	@mkdir -p $(dir $@)
	@if [ "$(NEW_COMPILE_FLAGS)" != "$(OLD_COMPILE_FLAGS)" ] ; then \
		echo "GEN $@"; \
		echo "$(NEW_COMPILE_FLAGS)" > $@; \
	fi

# Clean the build/ and submission/ dirs.
clean:
	@rm -rf .cache/ build/ submission/ /tmp/restaurant.pid

# Run ./bootstrap.sh
run:
	./src/bootstrap.sh $(ARGS)

# Copy files in a submission subdir with the layout that professors expect.
ALL_SOURCES = $(shell find src/ -type f)
submission: easy.mk .env.example $(ALL_SOURCES)
	@rm -rf submission/
	@mkdir submission/
	@cp -vr .env.example submission/.env
	@cp -vr easy.mk submission/Makefile
	@cp -vr src/status.sh submission/status.sh
	@sed -i 's|src/lib/result.sh|code/lib/result.sh|' submission/status.sh
	@cp -vr src/bootstrap.sh submission/bootstrap.sh
	@sed -i 's|src/lib/result.sh|code/lib/result.sh|' submission/bootstrap.sh
	@cp -vr src/ submission/code/ -r
	@rm submission/code/bootstrap.sh
	@rm submission/code/status.sh
	@cp report.pdf submission/

format: clang-format/fix
	npx prettier -w .

clang-format/fix:
	@clang-format -i $(SOURCES)
clang-format/check:
	@clang-format --dry-run --Werror $(SOURCES)
clang-tidy/check:
	@clang-tidy -p build/ --use-color $(SOURCES)

# Used to force file-generating targets to always run.
ALWAYS:
.PHONY: clean run format clang-format/fix clang-format/check clang-tidy/check ALWAYS
