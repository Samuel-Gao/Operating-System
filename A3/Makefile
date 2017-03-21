ext2_ls : ext2_ls.o helper.o
	gcc -Wall -Werror -g -o ext2_ls ext2_ls.o helper.o

%.o: %.c 
	gcc -Wall -g -c $<

clean:
	rm -f ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm read *.o *~