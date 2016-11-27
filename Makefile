TGT= rainflow
OBJECTS=
CFLAGS= -march=native -O2 -pipe -std=c11
CXXFLAGS= -march=native -O2 -pipe -std=c++11
CC= gcc
CXX= g++
#TESTS= sine100hz.CSV sine1hz.CSV TH_Amplitudes.ASC

all: ${TGT}

${TGT}:

.PHONY: clean test

clean:
	$(RM) $(TGT) $(TGT).out

test: ${TGT}
	./rainflow sine100hz.CSV >/dev/null
	diff rainflow.out sine100hz_golden.out
	./rainflow sine1hz.CSV >/dev/null
	diff rainflow.out sine1hz_golden.out
	./rainflow TH_Amplitudes.ASC >/dev/null
	diff rainflow.out TH_Amplitudes_golden.out