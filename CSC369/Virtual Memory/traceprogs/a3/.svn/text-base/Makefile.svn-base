LOGFILE='"gc.log"'
FLAGS = -DLOGFILE=${LOGFILE} -Wall -g 

all : mktrans do_trans do_fstree test_gc

do_trans : do_trans.o list.o gc.o gc_list.o
	gcc ${FLAGS} -o $@ $^

mktrans : mktrans.o list.o gc.o gc_list.o
	gcc ${FLAGS} -o $@ $^

do_fstree : do_fstree.o fstree.o gc.o gc_fstree.o
	gcc ${FLAGS} -o $@ $^

test_gc : test_gc.o list.o gc.o gc_list.o
	gcc ${FLAGS} -o $@ $^

gc_list : gc_list.o gc.o list.o
	gcc ${FLAGS} -o $@ $^

gc_fstree : gc_fstree.o gc.o fstree.o
	gcc ${FLAGS} -o $@ $^

%.o : %.c 
	gcc ${FLAGS} -c $<

do_trans.o : list.h gc.h
mktrans.o : list.h
list.o : list.h
fstree.o : fstree.h gc.h
do_fstree.o : fstree.h
gc.o : gc.h
test_gc.o : gc.h list.h gc_list.h
gc_list : gc.h list.h gc_list.h
gc_fstree : gc.h fstree.h gc_fstree.h

clean : 
	rm *.o mktrans do_trans do_fstree test_gc_list
