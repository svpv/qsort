RPM_OPT_FLAGS ?= -O2 -g -Wall
all: bench
bench: bench.cc qsort.h mjt.h
	$(CXX) $(RPM_OPT_FLAGS) -fwhole-program -o $@ $<
	./$@
mjt.h:
	wget -O $@ http://www.corpit.ru/mjt/qsort/qsort.h
