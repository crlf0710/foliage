# make native build by default

include prj/shared/preamble.cfg

%:
	@$(MAKE) --no-print-directory -C prj/native/gcc $@
