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

/* times.c
   Time conversion functions
*/

#define INCL_NOPMAPI
#define INCL_DOSDATETIME
#include <os2.h>

#include <time.h>

FDATE unix_to_os2_date(time_t unix_time)
{
  struct tm *local = localtime(&unix_time);
  FDATE d;

  d.day = local->tm_mday;
  d.month = local->tm_mon+1;
  d.year = local->tm_year - 80;

  return d;
}

FTIME unix_to_os2_time(time_t unix_time)
{
  struct tm *local = localtime(&unix_time);
  FTIME t;

  t.twosecs = local->tm_sec/2;
  t.minutes = local->tm_min;
  t.hours = local->tm_hour;

  return t;
}

time_t os2_to_unix_time(FDATE d, FTIME t)
{
  time_t unix_time;
  struct tm local;

  local.tm_mday = d.day;
  local.tm_mon = d.month-1;
  local.tm_year = d.year+80;
  local.tm_sec = 2*t.twosecs;
  local.tm_min = t.minutes;
  local.tm_hour = t.hours;
  unix_time=mktime(&local)+timezone;

  return unix_time;
}
