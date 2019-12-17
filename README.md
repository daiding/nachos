# Nachos

Operating System Course Project

Chen Haijian @ 2019

# Building

## Environment

- GCC 4.1.2+ with i686 arch support

- GCC-Multilib for x86-64 based systems

## Run

```bash
$ cd code
$ make 
$ cd threads
$ ./nachos
```

# Install requirements for x86-64 based Linux

## Debian-based Linux

```bash
$ sudo apt-get install gcc-multilib g++-multilib lib32stdc++6
```

Tested in Ubuntu 14.04  / Ubuntu 18.04 / Linux Mint 19.2

## Windows Subsystem for Linux

You need a QEMU to run 32-bit ELF programs, since WSL doesn't support 32-bit ELF programs natively.

In Ubuntu 18.04:

```bash
sudo apt update
sudo apt install qemu-user-static
sudo update-binfmts --install i386 /usr/bin/qemu-i386-static --magic '\x7fELF\x01\x01\x01\x03\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x03\x00\x01\x00\x00\x00' --mask '\xff\xff\xff\xff\xff\xff\xff\xfc\xff\xff\xff\xff\xff\xff\xff\xff\xf8\xff\xff\xff\xff\xff\xff\xff'
```

Run
```bash
sudo service binfmt-support
```
every time you need to run 32-bit ELF programs for current bash instance.

I found mips-xgcc still can't work properly, though.

## Install newer version of gcc from source in the course server

```bash
# Maybe failed, since `wget` in the course server is too old, you can use scp to upload files instead
$ wget --no-check-certificate https://ftp.gnu.org/gnu/gcc/gcc-4.9.4/gcc-4.9.4.tar.gz
$ tar -xzf gcc-4.9.4.tar.gz
$ cd gcc-4.9.4
$ ./contrib/download_prerequisites
$ ./configure --enable-checking=release --enable-languages=c,c++ --enable-multilib --prefix=$HOME/env/
$ make -j4
$ make install 
```

Open ~/.bash_profile, add

```bash
export LD_LIBRARY_PATH=~/env/lib64:~/env/lib:~/env/libexec:$LD_LIBRARY_PATH
export LD_RUN_PATH=~/env/lib64:~/env/lib:~/env/libexec:$LD_RUN_PATH
export PKG_CONFIG_PATH=~/env/lib64/pkgconfig:~/env/lib/pkgconfig:$PKG_CONFIG_PATH
```
Run
```
$ source ~/.bash_profile
```

You can also install gdb, astyle, or vim as you like :-D

Tested in CentOS 5.11 (Course server)


# Difference compared to the original version

This version of nachos comes from https://github.com/h8liu/nachos with some modification of Makefile and coff2noff.c to fit Linux and silence warnings.

Compared to the original version of nachos 3.4 in https://homes.cs.washington.edu/~tom/nachos/, this version

1. Fixed gcc warnings caused by non-standard C++ code

2. Fixed a delete[] bug in ./code/userprog/addrspace.cc 

3. **Modified function Exec() in ./code/userprog/syscall.h**

Before

```cpp
SpaceId Exec(char *name)
```

Afer

```cpp
SpaceId Exec(char *name, int argc, char** argv, int opt)
```

4. Formatted code using astyle

5. Modified makefile, now you can use

```bash
$ cd code
$ make fmt # Format code using astyle
$ make ubuntu # Install requirements for Debian-based Linux
```

6. Use mips-xgcc to compile nachos userspace programs (It won't cause error compared to the course server's version)


# License

Read ./COPYRIGHT for details.