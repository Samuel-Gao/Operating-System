all : ext2_ls ext2_cp ext2_mkdir ext2_rm ext2_ln ext2_rm_bonus

ext2_ls : ext2_ls.o helper.o
	gcc -Wall -Werror -g -o ext2_ls ext2_ls.o helper.o

ext2_cp : ext2_cp.o helper.o
	gcc -Wall -Werror -g -o ext2_cp ext2_cp.o helper.o

ext2_mkdir : ext2_mkdir.o helper.o
	gcc -Wall -Werror -g -o ext2_mkdir ext2_mkdir.o helper.o

ext2_rm : ext2_rm.o helper.o
	gcc -Wall -Werror -g -o ext2_rm ext2_rm.o helper.o

ext2_rm_bonus : ext2_rm_bonus.o helper.o
	gcc -Wall -Werror -g -o ext2_rm_bonus ext2_rm_bonus.o helper.o

ext2_ln : ext2_ln.o helper.o
	gcc -Wall -Werror -g -o ext2_ln ext2_ln.o helper.o


%.o: %.c 
	gcc -Wall -g -c $<

clean:
	rm -f ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm ext2_rm_bonus read *.o *~