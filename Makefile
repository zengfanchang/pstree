NAME := $(shell basename $(PWD))
export MODULE := M1
all: $(NAME)-64 $(NAME)-32
gcc_args := -ggdb
gdb_args := -q
cmd_gdb := ./pstree.gdb
input_args := -p --show-pids --version


test: pstree.c
	gcc pstree.c -o pstree-test
	./pstree-test $(input_args) 

debugger: pstree.c
	gcc $< -o pstree-debug $(gcc_args)
	gdb $(gdb_args) -x $(cmd_gdb) --args pstree-debug $(input_args)
	
.PHONY: clean ALL
clean:
	rm -rf ./pstree-test ./pstree-debug
