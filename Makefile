RM=/bin/rm
SUBDIRS=src

LDFLAGS=-Wall -ggdb
CFLAGS=-Wall -pedantic -ggdb -ansi --std=c99

MAKE=make 'CFLAGS=${CFLAGS}' 'LDFLAGS=${LDFLAGS}'

all: build

build:
	@for i in $(SUBDIRS); do \
                echo "Building $$i";\
                cd $$i;\
                ${MAKE} build; cd ..;\
        done

clean:
	${RM} -f *~ core *.core
	@for i in $(SUBDIRS); do \
                echo "Cleaning $$i";\
                cd $$i;\
                ${MAKE} clean; cd ..;\
        done

distclean: clean
	${RM} -f slowlane
