/*****************************************************************************
 * broacast.h : SAP Broadcast Class definition
 ****************************************************************************
 * Copyright (C) 1998-2005 VideoLAN
 * $Id: broadcast.h 277 2005-06-08 11:56:17Z courmisch $
 *
 * Authors: Damien Lucas <nitrox@videolan.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#define HELLO_PORT 9875
#define HELLO_PORT_STR "9875"
#define SAP_IPV4_ADDR "224.2.127.254"
#define SAP_IPV6_ADDR_1 "ff0"
/* You must put the scope in between */
#define SAP_IPV6_ADDR_2 "::2:7ffe"


class Broadcast {
  public:
    Broadcast(int i_ttl = 0, const char *psz_iface = NULL);
    ~Broadcast(void);
    int Send(Message*, const struct sockaddr *dst, socklen_t len);

    int GuessDestination(const char *prgm, struct sockaddr_storage *addr,
                         socklen_t *len) const;

  private:
    int fd4, fd6;                   /* File descriptor on the socket */
    int scope_id;
};
