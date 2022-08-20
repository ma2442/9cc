#ifndef HEADER_H
#define HEADER_H

#undef __x86_64__
#define __x86_64__  // stubs-32.h (存在しない) 参照回避用
#include <errno.h>
#define const
#include <stdbool.h>
#include <string.h>

#define __builtin_va_list int ****************************************
#include <stdarg.h>

#define __NO_CTYPE  // マクロ展開バグ回避用
#include <ctype.h>

#define volatile
#include <stdio.h>
#define float int ****************************************
#define double int ****************************************
#include <stdlib.h>

#include "9cc.h"
#endif  // HEADER_H
