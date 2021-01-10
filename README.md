# Flexible File Operation tester

FFO can execute file operations against specified files. It is useful for file system debugging.

## Build

make

## Run

./ffo

## Usage

For more detailed descriptions, try the `m` command.

### Simple usage

```
[root@localhost ffo]# ./ffo 
Welcome to use flexible file operator.
-------------------------------------------
Command (m for help): a op=open;filename=testfile;openflags=creat
Open() return with 3
-------------------------------------------
Command (m for help): a op=pwrite;fd=3;rwcount=4096;offset=0
pwrite(fd=3) return with 4096
-------------------------------------------
Command (m for help): a op=pread;fd=3;rwcount=4096;offset=0
pread(fd=3) return with 4096
-------------------------------------------
Command (m for help): a op=close;fd=3
Close(fd=3) return with 0
-------------------------------------------
Command (m for help): q
Bye

```

### Use a register to pass the fd argument

If you don't want to specify the fd by yourself (e.g., you want to execute a sequence of commands and you don't know fd before running), just save the fd to a register after open() and then pass the register to the later commands.

Range of the register number: [1, 16]

```
[root@localhost ffo]# ./ffo 
Welcome to use flexible file operator.
-------------------------------------------
Command (m for help): a op=open;fn=testfile;of=creat;fdr=1
Open() return with 3
-------------------------------------------
Command (m for help): a op=write;fdr=1;rwcount=8
write(fd=3) return with 8
-------------------------------------------
Command (m for help): a op=fdatasync;fdr=1;fdr=1
fdatasync(fd=3) return with 0
-------------------------------------------
Command (m for help): a op=close;fdr=1
Close(fd=3) return with 0
-------------------------------------------
Command (m for help): q
Bye

```

### Run a sequence of commands

TODO

### Run a script

TODO
