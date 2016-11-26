default: all

node: all node-all

node-all:
	cd ext/node && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
	cd ext/node && $(MAKE) $@

.DEFAULT:
	cd src && $(MAKE) $@
