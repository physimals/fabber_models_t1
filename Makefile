include ${FSLCONFDIR}/default.mk

PROJNAME = fabber_t1
LIBS = -lfsl-fabberexec -lfsl-fabbercore -lfsl-newimage \
       -lfsl-miscmaths -lfsl-utils -lfsl-cprob \
       -lfsl-NewNifti -lfsl-znz -ldl
XFILES = fabber_t1
SOFILES = libfsl-fabber_models_t1.so

# Forward models
OBJS =  fwdmodel_vfa.o

# For debugging:
#OPTFLAGS = -ggdb

# Pass Git revision details
GIT_SHA1:=$(shell git describe --dirty)
GIT_DATE:=$(shell git log -1 --format=%ad --date=local)
CXXFLAGS += -DGIT_SHA1=\"${GIT_SHA1}\" -DGIT_DATE="\"${GIT_DATE}\""

#
# Build
#

all: ${XFILES} ${SOFILES}

# models in a library
libfsl-fabber_models_t1.so : ${OBJS}
	${CXX} ${CXXFLAGS} -shared -o $@ $^ ${LDFLAGS}

# fabber built from the FSL fabbercore library including the models specifieid in this project
fabber_t1 : fabber_client.o libfsl-fabber_models_t1.so
	${CXX} ${CXXFLAGS} -o $@ $< -lfsl-fabber_models_t1 ${LDFLAGS}
