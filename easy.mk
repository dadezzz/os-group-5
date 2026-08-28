SOURCES  = $(shell find code/ -name '*.c')
OBJECTS  = $(patsubst code/%.c,build/objects/%.o,$(SOURCES))
DEPS     = $(patsubst %.o,%.d,$(OBJECTS))

all: build

# Generate .d files to recompile when header files change.
CPPFLAGS += -MMD -MP
# Set C standard to 23 and enable some stricter diagnostics.
CFLAGS   += -std=gnu23 -pthread
# Add libm for ceil and pow functions.
LDFLAGS  += -pthread -lm

# Include the header files generated with -MMD in $CPPFLAGS.
-include $(DEPS)

build: build/restaurant

# Link the restaurant executable.
build/restaurant: $(OBJECTS)
	@echo "LD $@"
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# Create object and compile_commands files for each source file.
build/objects/%.o: code/%.c
	@echo "CC $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Clean the build/ dir and tmp pid file.
clean:
	@rm -rf build/ /tmp/restaurant.pid

# Run ./bootstrap.sh, optionally with args passed via ARGS
run:
	@./bootstrap.sh $(ARGS)

.PHONY: clean run
