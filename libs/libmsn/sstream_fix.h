/*
 * This file is one big nasty kludge because the older
 * libstdc++ libraries (before v3) have the old strstream
 * but I want to use the new, and much improved sstream.
 * - Barnaby
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SSTREAM
# include <sstream>
#elif HAVE_STRSTREAM
# define USE_STRSTREAM_WRAPPERS
#else
# error "No sstream/strstream implementation"
#endif

#if defined(USE_STRSTREAM_WRAPPERS) && !defined(SSTREAM_FIX_H)
#define SSTREAM_FIX_H

#include <string>
#include <strstream>

#include "libmsn_export.h"

namespace std
{

  /*
   * Only limited functionality from ostringstream
   * is implemented
   */
  class ostringstream : public ostrstream {
   public:
    string str() {
      char *cstr = ostrstream::str();
      freeze(0);
      if (cstr == 0) return string();
      return string(cstr,pcount());
    }
  };

  /*
   * Only limited functionality from istringstream
   * is implemented
   */
  class istringstream : public istrstream {
   public:
    istringstream(const string& str)
      : istrstream(str.c_str()) { }
  };

}

#endif
