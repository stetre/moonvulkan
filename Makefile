
default: build

build install uninstall where:
	@cd src;		$(MAKE) $@

clean :
	@cd src;		$(MAKE) $@
	@cd doc;		$(MAKE) $@
	@cd examples/allocator;	$(MAKE) $@

docs:
	@cd doc;		$(MAKE)

cleanall: clean

backup: clean
