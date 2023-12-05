typedef unsigned char      U8;
typedef signed char        S8;
typedef unsigned short     U16;
typedef signed short       S16;
typedef unsigned long      U32;
typedef signed long        S32;
typedef unsigned long long U64;
typedef signed long long   S64;

typedef enum {
   FALSE = 0,
   OFF   = 0,
   TRUE  = 1,
   ON    = 1
} Bool;

typedef enum {
   XFALSE = 0,
   XTRUE  = 1,
   XERROR = 2,
   XNA    = 3
} XBool;

#ifndef NULL
   #define NULL	0
#endif
