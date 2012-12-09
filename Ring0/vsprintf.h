/* vsprintf.h
*/

#ifndef _VSPRINTF_H
#define _VSPRINTF_H

#include <stdarg.h>

int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif /* _VSPRINTF_H */
