/*
 * HFS/2 - A Hierarchical File System Driver for OS/2
 * Copyright (C) 1996, 1997 Marcus Better
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* names.c
   Filename translation functions.
*/

#include <string.h>
#include <ctype.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "names.h"
#include "trans.h"

/* Global variable set by command line option */
int filename_translation = FN_TR_CP850;

unsigned char quote_char = '%';

static int mac_to_os2_name_cp850(const unsigned char *, unsigned char *, int);
static int os2_to_mac_name_cp850(const unsigned char *, unsigned char *);
static int mac_to_os2_name_7bit(const unsigned char *, unsigned char *, int);
static int os2_to_mac_name_7bit(const unsigned char *, unsigned char *);
static int mac_to_os2_name_8bit(const unsigned char *, unsigned char *, int);
static int os2_to_mac_name_8bit(const unsigned char *, unsigned char *);
static unsigned char to_hex(int);
int os2_invalid(unsigned char);
int os2_invalid_wild(unsigned char);

/* Translate Macintosh filename to OS/2 filename. Nonzero return value means filename
   is invalid. */
int mac_to_os2_name(const unsigned char *src, unsigned char *dest)
{
  switch(filename_translation) {
  case FN_TR_CP850:
    return mac_to_os2_name_cp850(src, dest, 0);
  case FN_TR_7BIT:
    return mac_to_os2_name_7bit(src, dest, 0);
  case FN_TR_8BIT:
    return mac_to_os2_name_8bit(src, dest, 0);
  default:
#ifdef DEBUG
    printf("Unhandled filename translation!\n");
#endif
    return 1;
  }
}

/* Same as mac_to_os2_name(), but wildcard characters are allowed. */
int mac_to_os2_name_wild(const unsigned char *src, unsigned char *dest)
{
  switch(filename_translation) {
  case FN_TR_CP850:
    return mac_to_os2_name_cp850(src, dest, 1);
  case FN_TR_7BIT:
    return mac_to_os2_name_7bit(src, dest, 1);
  case FN_TR_8BIT:
    return mac_to_os2_name_8bit(src, dest, 1);
  default:
#ifdef DEBUG
    printf("Unhandled filename translation!\n");
#endif
    return 1;
  }
}

/* Translate OS/2 filename to Macintosh filename. Nonzero return value means 
   filename is invalid. */
int os2_to_mac_name(const unsigned char *src, unsigned char *dest)
{
  switch(filename_translation) {
  case FN_TR_CP850:
    return os2_to_mac_name_cp850(src, dest);
  case FN_TR_7BIT:
    return os2_to_mac_name_7bit(src, dest);
  case FN_TR_8BIT:
    return os2_to_mac_name_8bit(src, dest);
  default:
#ifdef DEBUG
    printf("Unhandled filename translation!\n");
#endif
    return 1;
  }
}

static int mac_to_os2_name_cp850(const unsigned char *src, unsigned char *dest,
                                 int wildcards)
{
  int i, j;
  char *p=dest;

  for(i=j=0; i<strlen(src); i++) {
    unsigned char c=src[i];
    if(c==MAC_PATH_SEPARATOR)
      *p++ = OS2_PATH_SEPARATOR;
    else if(c>=32 && c<=127 && 
            !(wildcards ? os2_invalid_wild(c) : os2_invalid(c)) && 
            c!=quote_char)
      *p++ = c;
    else if(c>=128 && mac_to_cp850(c))
      *p++ = mac_to_cp850(c);
    else {
      *p++ = quote_char;
      *p++ = to_hex(c/16);
      *p++ = to_hex(c%16);
    }
  }
  *p=0;
  return 0;
}

static int os2_to_mac_name_cp850(const unsigned char *src, unsigned char *dest)
{
  static int table_generated;
  int i, j;
  unsigned char *p=dest, *component=dest;

  /* Generate reverse mapping first time */
  if(!table_generated) {
    int i;
    for(i=0; i<128; i++)
      if(mac_to_cp850_map[i])
	cp850_to_mac_map[mac_to_cp850_map[i]-128]=i+128;
    table_generated=1;
  }

  for(i=j=0; i<strlen(src) && p-component<=MAX_MAC_FN_LEN; i++) {
    unsigned char c=src[i];
    if(c==OS2_PATH_SEPARATOR) {
      *p++ = MAC_PATH_SEPARATOR;
      component = p;
    }
    else if(c==quote_char) {
      if(strlen(src+i+1)<2)
	return 1;
      if(isxdigit(src[i+1]) && isxdigit(src[i+2])) {
	int j, n=0;
	for(j=1; j<=2; j++) {
	  unsigned char ch=src[i+j];
	  n <<= 4;
	  if(isalpha(ch))
	    n+=toupper(ch)-'A'+10;
	  else
	    n+=ch-'0';
	}
	if(n==quote_char || os2_invalid(n))
	  *p++ = n;
	else if(n>=128 && !cp850_to_mac(n))
	  *p++ = n;
	else
	  return 1;
	i+=2;
      }
      else
	return 1;
    }
    else if(c>=32 && c<=127)
      *p++ = c;
    else if(c>=128)
      if(cp850_to_mac(c))
	*p++ = cp850_to_mac(c);
      else
	return 1;
    else
      return 1;
  }
  *p=0;
  if(i==strlen(src))
    return 0;
  else
    return 1;
}

static int mac_to_os2_name_7bit(const unsigned char *src, unsigned char *dest,
				int wildcards)
{
  int i, j;
  char *p=dest;

  for(i=j=0; i<strlen(src); i++) {
    unsigned char c=src[i];
    if(c==MAC_PATH_SEPARATOR)
      *p++ = OS2_PATH_SEPARATOR;
    else if(c>=32 && c<=127 && 
            !(wildcards ? os2_invalid_wild(c) : os2_invalid(c)) && 
            c!=quote_char)
      *p++ = c;
    else {
      *p++ = quote_char;
      *p++ = to_hex(c/16);
      *p++ = to_hex(c%16);
    }
  }
  *p=0;
  return 0;
}

static int os2_to_mac_name_7bit(const unsigned char *src, unsigned char *dest)
{
  int i, j;
  unsigned char *p=dest, *component=dest;

  for(i=j=0; i<strlen(src) && p-component<=MAX_MAC_FN_LEN; i++) {
    unsigned char c=src[i];
    if(c==OS2_PATH_SEPARATOR) {
      *p++ = MAC_PATH_SEPARATOR;
      component = p;
    }
    else if(c==quote_char) {
      if(strlen(src+i+1)<2)
	return 1;
      if(isxdigit(src[i+1]) && isxdigit(src[i+2])) {
	int j, n=0;
	for(j=1; j<=2; j++) {
	  unsigned char ch=src[i+j];
	  n <<= 4;
	  if(isalpha(ch))
	    n+=toupper(ch)-'A'+10;
	  else
	    n+=ch-'0';
	}
	if(n==quote_char || os2_invalid(n) || n>=128)
	  *p++ = n;
	else
	  return 1;
	i+=2;
      }
      else
	return 1;
    }
    else if(c>=32 && c<=127)
      *p++ = c;
    else
      return 1;
  }
  *p=0;
  if(i==strlen(src))
    return 0;
  else
    return 1;
}

static int mac_to_os2_name_8bit(const unsigned char *src, unsigned char *dest,
				int wildcards)
{
  int i, j;
  char *p=dest;
  
  for(i=j=0; i<strlen(src); i++) {
    unsigned char c=src[i];
    if(c==MAC_PATH_SEPARATOR)
      *p++ = OS2_PATH_SEPARATOR;
    else if(c>=32 && 
            !(wildcards ? os2_invalid_wild(c) : os2_invalid(c)) && 
            c!=quote_char)
      *p++ = c;
    else {
      *p++ = quote_char;
      *p++ = to_hex(c/16);
      *p++ = to_hex(c%16);
    }
  }
  *p=0;
  return 0;
}

static int os2_to_mac_name_8bit(const unsigned char *src, unsigned char *dest)
{
  int i, j;
  unsigned char *p=dest, *component=dest;

  for(i=j=0; i<strlen(src) && p-component<=MAX_MAC_FN_LEN; i++) {
    unsigned char c=src[i];
    if(c==OS2_PATH_SEPARATOR) {
      *p++ = MAC_PATH_SEPARATOR;
      component = p;
    }
    else if(c==quote_char) {
      if(strlen(src+i+1)<2)
	return 1;
      if(isxdigit(src[i+1]) && isxdigit(src[i+2])) {
	int j, n=0;
	for(j=1; j<=2; j++) {
	  unsigned char ch=src[i+j];
	  n <<= 4;
	  if(isalpha(ch))
	    n+=toupper(ch)-'A'+10;
	  else
	    n+=ch-'0';
	}
	if(n==quote_char || os2_invalid(n))
	  *p++ = n;
	else
	  return 1;
	i+=2;
      }
      else
	return 1;
    }
    else if(c>=32)
      *p++ = c;
  }
  *p=0;
  if(i==strlen(src))
    return 0;
  else
    return 1;
}

static unsigned char to_hex(int i)
{
  if(i<10)
    return i+'0';
  else
    return i-10+'a';
}

/* Determine if filename is FAT 8.3 format */
unsigned is_short_filename(char *name)
{
  const char FAT_special_chars[]="$%'-_@{}~`!#()";
  int i, j, k;

  /* Check name */
  for( i=j=k=0; i<strlen(name) && name[i]!='.'; i++ ) {
    if(!isalnum(name[i]) && !strchr(FAT_special_chars, name[i]) && name[i]!=' ')
      return 0;

    if(name[i]==' ')
      k++;
    else if(k>0) {
      j+=k;
      k=0;
      j++;
    }
    else
      j++;
  }
  if(j>8)
    return 0;

  if(i==strlen(name))
    return 1;
  i++;

  /* Check extension */
  for( j=k=0; i<strlen(name); i++ ) {
    if(!isalnum(name[i]) && !strchr(FAT_special_chars, name[i]) && name[i]!=' ')
      return 0;

    if(name[i]==' ')
      k++;
    else if(k>0) {
      j+=k;
      k=0;
      j++;
    }
    else
      j++;
  }
  if(j>3)
    return 0;

  return 1;
}

/* Test if character is invalid in OS/2 filenames */
int os2_invalid(unsigned char c)
{
  return strchr(os2_invalid_chars, (c)) || (c<32);
}

/* Test if character is invalid in OS/2 filenames, but allowing wildcards */
int os2_invalid_wild(unsigned char c)
{
  if(c=='*' || c=='?')
    return 0;
  else
    return os2_invalid(c);
}
