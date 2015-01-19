# platform configurations

include ../../shared/preamble.cfg

export PATH    := /opt/bin:$(PATH)
export CC      := i686-w64-mingw32-gcc.exe
export CXX     := i686-w64-mingw32-g++.exe
export WINDRES := i686-w64-mingw32-windres.exe

%:
	@$(MAKE) --no-print-directory -C ../../shared $@
