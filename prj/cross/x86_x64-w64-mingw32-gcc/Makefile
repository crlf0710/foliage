# platform configurations

include ../../shared/preamble.cfg

export PATH    := /opt/bin:$(PATH)
export CC      := x86_64-w64-mingw32-gcc.exe
export CXX     := x86_64-w64-mingw32-g++.exe
export WINDRES := x86_64-w64-mingw32-windres.exe

%:
	@$(MAKE) --no-print-directory -C ../../shared $@
