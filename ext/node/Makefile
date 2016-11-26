lib_dir := $(shell sh -c "pwd | sed 's/\//\\\\\//g'")\/..\/..\/src

node-all:
	sed -i '.bak' "s/#LIB_DIR#/$(lib_dir)/" binding.gyp
	node-gyp configure build

clean:
	rm -rf build
