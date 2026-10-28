#ifndef RIFT_LAUNCH_PATH_H
#define RIFT_LAUNCH_PATH_H
/* The kernel passes a byte-counted block of NUL-separated argument strings.
   Capture argv[0] before the module entry returns. No heap or CRT is used. */
#define RIFT_LAUNCH_PATH_MAX 256
#define RIFT_LAUNCH_DIR_MAX 192
int rift_launch_capture(unsigned bytes,const void *argp);
const char *rift_launch_executable(void);
const char *rift_launch_directory(void);
const char *rift_launch_boot_log(void);
int rift_launch_last_io_error(void);
int rift_launch_open_package(void);
#endif
