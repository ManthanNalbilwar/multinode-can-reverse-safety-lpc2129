// Defines short and readable data type names used throughout the project.
// FIX: include guard added. Earlier, MAIN_NODE.c included this file and then
//      can_major.h included it again -> "typedef redefinition" warning/error.
#ifndef MAJOR_TYPE_H
#define MAJOR_TYPE_H

typedef unsigned char      u8;
typedef char               s8;     // plain char on purpose: string literals are "char *"
typedef unsigned short int u16;
typedef signed short int   s16;
typedef unsigned int       u32;
typedef int                s32;
typedef float              f32;
typedef double             d64;

#endif
