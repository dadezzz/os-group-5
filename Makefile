all: build
.PHONY: build,clean,run,package

## User targets ##

build: build/restaurant

# Cleans the build/ and output/ dirs.
clean:
	@ rm -rf build/ output/

# Runs ./bootstrap.sh
#
# TODO: read (optionally) args to pass from the ARGS variable.
run:
	./bootstrap.sh

# Runs ./status.sh
status:
	./status.sh

# Packages the project as required by the professors.
package: output/

## Internal targets ##

sources = $(shell find src/ -name *.c)
objects = $(patsubst src/%.c,build/objects/%.o,$(sources))

# Links all built object files.
build/restaurant: $(objects)
	@ echo "LD $@"
	@ $(CC) $(LDFLAGS) -o build/restaurant $(objects)

build/objects/%.o: src/%.c
	@ echo "CC $@"
	@ mkdir -p $(dir $@)
	@ $(CC) $(CFLAGS) -c $< -o $@

output/: Makefile src/ bootstrap.sh status.sh
	@ rm -rf output/
	@ mkdir -p output/code/
	@ cp -vr Makefile bootstrap.sh status.sh src/ output/code/
	@ # cp report.pdf output

