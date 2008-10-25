/***************************************************************************
 *  19/06/2008
 *  Copyright  2008  wolfgar
 *  Email
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*!
 * \file debug.h
 * \brief debug macro definiton
 * \author wolfgar
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <directfb.h>

#ifdef DEBUG
#include <stdio.h>
/**
 * \def PRINTD
 * \brief Debug macro without format
 */
#define PRINTD(s) fprintf (stderr, (s) )
/**
 * \def PRINTDF
 * \brief Debug macro with format
 */
#define PRINTDF(s, ...) fprintf (stderr, (s), __VA_ARGS__)
#else
#define PRINTD(s)
#define PRINTDF(s, ...)
#endif

/**
 * \def DFBCHECK
 * \brief macro for a safe call to DirectFB functions
 */
#define DFBCHECK(x...)                                                    \
     {                                                                    \
          err = x;                                                        \
          if (err != DFB_OK) {                                            \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );     \
               DirectFBErrorFatal( #x, err );                             \
          }                                                               \
     }

#endif /* __DEBUG_H__ */
