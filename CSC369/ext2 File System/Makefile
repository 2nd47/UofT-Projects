
all :  ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm

ext2_ls :  ext2_ls.o ext2_shared.o

ext2_cp :  ext2_cp.o ext2_shared.o

ext2_mkdir : ext2_mkdir.o ext2_shared.o

ext2_ln : ext2_ln.o ext2_shared.o

ext2_rm : ext2_rm.o ext2_shared.o

%.o : %.c ext2.h
	gcc -Wall -g -c $<

clean : reset
	rm -f *.o all *~

reset : emptydisk twolevel largefile

emptydisk :
	rm -f emptydisk.img
	cp imgbkup/emptydisk.img .

twolevel :
	rm -f twolevel.img
	cp imgbkup/twolevel.img .
   
largefile :
	rm -f largefile.img
	cp imgbkup/largefile.img .


FILENAME_EXT := making-large-name-to-make-the-directory-entry-very-long-so-that-the-directory-inode-must-contain-two-blocks-instead-of-one-such-that-we-can-properly-test-the-file-removal-therefore-we-know-that-in-the-case-of-a-two-block-directory.txt

test : all reset removetest mount

removetest : 
	rm -f -r mktest
	rm -f -r tmp

unmount : 
	fusermount -u tmp

createdirs : 
	mkdir mktest
	@for n in $(shell seq 1 6); do\
		echo "creating dir $$n" ; \
		ext2_mkdir twolevel.img /level1/$$n ; \
	done

createfiles : createdirs
	@for n in $(shell seq 1 5); do\
		echo "creating file on native system $$n" ; \
		dd if=/dev/zero of=mktest/mktest-$$n-$(FILENAME_EXT) bs=15K count=1 ; \
		echo "creating readable content on to file $$n" ; \
		echo "hello world" > mktest/mktest-$$n-$(FILENAME_EXT) ; \
		for i in $(shell seq 1 10); do\
			cat mktest/mktest-$$n-$(FILENAME_EXT) mktest/mktest-$$n-$(FILENAME_EXT) > mktest/tmpfile && mv mktest/tmpfile mktest/mktest-$$n-$(FILENAME_EXT) ; \
		done; \
		for i in $(shell seq 1 200); do\
			echo "hello world" >> mktest/mktest-$$n-$(FILENAME_EXT) ; \
		done; \
		echo "copying file to image $$n" ; \
		ext2_cp  twolevel.img mktest/mktest-$$n-$(FILENAME_EXT) /level1; \
	done; \
	rm -f mktest/tmpfile ; \

createlinks : createfiles
	@for n in $(shell seq 1 5); do\
		echo "creating link to file $$n" ; \
		ext2_ln twolevel.img /level1/mktest-$$n /ln$$n ; \
		echo "created link" ; \
	done

removefiles_links : createlinks
	@for n in $(shell seq 1 2 5); do\
		echo "removing link $$n" ; \
		ext2_rm twolevel.img /ln$$n ; \
		echo "removed link" ; \
	done
	@for n in $(shell seq 2 2 5); do\
		echo "removing file $$n" ; \
		ext2_rm twolevel.img /level1/mktest-$$n-$(FILENAME_EXT) ; \
		echo "removed file" ; \
	done

createmore : removefiles_links
	@for n in $(shell seq 2 2 5); do\
		echo "creating file on native system $$n" ; \
		dd if=/dev/zero of=mktest/mktest-$$n-$(FILENAME_EXT) bs=15K count=1 ; \
		echo "creating readable content on to file $$n" ; \
		echo "hello world" > mktest/mktest-$$n-$(FILENAME_EXT) ; \
		for i in $(shell seq 1 5); do\
			cat mktest/mktest-$$n-$(FILENAME_EXT) mktest/mktest-$$n-$(FILENAME_EXT) > mktest/tmpfile && mv mktest/tmpfile mktest/mktest-$$n-$(FILENAME_EXT) ; \
		done; \
		for i in $(shell seq 1 200); do\
			echo "hello world" >> mktest/mktest-$$n-$(FILENAME_EXT) ; \
		done; \
		echo "copying file to image $$n" ; \
		ext2_cp  twolevel.img mktest/mktest-$$n-$(FILENAME_EXT) /level1; \
	done; \
	rm -f mktest/tmpfile

mount : createmore
	mkdir tmp
	fuseext2 -o rw+ twolevel.img tmp
	