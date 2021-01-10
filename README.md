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

### Run a sequence of commands

TODO

### Run a script

TODO
