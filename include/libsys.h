#pragma once

#include "./stdint.h"
#include "./krlibc.h"

// XJ380API POSIX Edition
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_STAT 4
#define SYS_FSTAT 5
#define SYS_LSTAT 6
#define SYS_POLL 7
#define SYS_LSEEK 8
#define SYS_MMAP 9
#define SYS_MPROTECT 10
#define SYS_MUNMAP 11
#define SYS_BRK 12

#define SYS_RT_SIGPROCMASK 14

#define SYS_IOCTL 16

#define SYS_READV 19
#define SYS_WRITEV 20

#define SYS_PIPE 22

#define SYS_MREMAP 25
#define SYS_MSYNC 26

#define SYS_DUP 32
#define SYS_DUP2 33
#define SYS_PAUSE 34
#define SYS_NANOSLEEP 35

#define SYS_ALARM 37

#define SYS_GETPID 39

#define SYS_SHUTDOWN 48

#define SYS_FORK 57
#define SYS_VFORK 58
#define SYS_EXECVE 59
#define SYS_EXIT 60
#define SYS_WAIT4 61
#define SYS_KILL 62
#define SYS_UNAME 63

#define SYS_GETDENTS 78
#define SYS_GETCWD 79
#define SYS_CHDIR 80
#define SYS_FCHDIR 81

#define SYS_MKDIR 83
#define SYS_RMDIR 84
#define SYS_CREAT 85
#define SYS_LINK 86
#define SYS_UNLINK 87
#define SYS_SYMLINK 88
#define SYS_READLINK 89
#define SYS_CHMOD 90
#define SYS_FCHMOD 91

#define SYS_MKNOD 133

#define SYS_ARCH_PRCTL 158

#define SYS_GETTID 186

#define SYS_SET_TID_ADDRESS 218

#define SYS_EXIT_GROUP 231

#define SYS_SET_ROBUST_LIST 273

#define SYS_RSEQ 334



// XJ380API XAPI Edition
#define XAPI_OFFEST 380

// P3.1
#define XAPI_OUTPUT    381
#define XAPI_INPUT     382

#define XAPI_GETLINE   418

#define XAPI_GETCH          383
#define XAPI_ENDLINE        384
#define XAPI_PRINTLINE      385
#define XAPI_PRINTF         386
#define XAPI_OUTPUT_SERIAL  427

// P3.2
#define XAPI_OPEN_FILE      387
#define XAPI_CLOSE_FILE     388

#define XAPI_SEARCH_FILE    416

#define XAPI_MAKEDIR        425
#define XAPI_CREATE_FILE    420
#define XAPI_DELETE_FILE    421  
#define XAPI_RENAME_FILE    422
#define XAPI_READ_FILE      423
#define XAPI_WRITE_FILE     424

// P3.4
#define XAPI_FORK   389
#define XAPI_EXECVE 390

// P3.5
#define XAPI_GET_VERSION        391

#define XAPI_GET_TIME           412
#define XAPI_GET_CURRENT_USER   413

#define XAPI_GET_TIME_X         433
#define XAPI_GET_CPU_MODEL      434
#define XAPI_GET_MEMORY_SIZE    435

// P3.6
#define XAPI_BROKEN         428
#define XAPI_SEND_APP_MSG   429
#define XAPI_SLEEP          430

// P4.1
#define XAPI_CREATE_WINDOW    392
#define XAPI_SET_WINDOW_TITLE 393
#define XAPI_CLOSE_WINDOW     394
#define XAPI_SET_ICON         395

#define XAPI_GET_WIN_SZIE     426

// P4.2
#define XAPI_DRAW_POINT       396
#define XAPI_DRAW_LINE        397
#define XAPI_DRAW_RECT        398
#define XAPI_DRAW_RECT_FILL   399
#define XAPI_DRAW_CIRCLE      400
#define XAPI_DRAW_CIRCLE_FILL 401
#define XAPI_DRAW_TEXT        402

#define XAPI_DRAW_TEXT_L      414
#define XAPI_DRAW_TEXT_SW     415
#define XAPI_CALC_TEXT_WIDTH  431

// P4.3
#define XAPI_DRAW_BMP       403
#define XAPI_DRAW_PNG       404

#define XAPI_DRAW_PICTURE   419

// P4.4
#define XAPI_SET_MSH_PROCOR 405

#define MSG_CHAR    0
#define MSG_MOVE    1
#define MSG_LBUTTON 2
#define MSG_RBUTTON 3
#define MSG_MBUTTON 4
#define MSG_ROLLER  5
#define MSG_CRL     6

// P4.5
#define XAPI_READBUFFER     406
#define XAPI_WRITEBUFFER    407
#define XAPI_READBUFFERA    417

#define XAPI_WRITEBUFFERA   408
#define XAPI_REFRESH_WINDOW 409

// P4.6
#define XAPI_BUTTON       410
#define XAPI_BUTTON_EMP   411
#define XAPI_DELETE_BUTTON  432

uint64_t enter_syscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);