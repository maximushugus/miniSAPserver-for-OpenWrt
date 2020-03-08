/*****************************************************************************
 * broadcast.cpp : SAP Broadcast class
 ****************************************************************************
 * Copyright (C) 1998-2006 VideoLAN
 * $Id: broadcast.cpp 362 2010-10-09 16:04:27Z courmisch $
 *
 * Authors: Damien Lucas <nitrox@videolan.org>
 *          Rémi Denis-Courmont <rem # videolan.org>
 *
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SOCKIO_H
# include <sys/sockio.h>    /* Needed on Solaris for SIOCGIFADDR */
#endif
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
using namespace std;

#include <vector>
#include <string>
#include "program.h"
#include "message.h"
#include "broadcast.h"

Broadcast::Broadcast(int i_ttl, const char *psz_iface) : fd4 (-1), fd6 (-1),
                                                         scope_id (0)
{
    /* Initializes IPv6 socket */
    if (psz_iface != NULL)
    {
        scope_id = if_nametoindex (psz_iface);
        if (scope_id == 0)
        {
            errno = ENODEV;
            perror (psz_iface);
            return;
        }
    }

    fd6 = socket (AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    while (fd6 != -1)
    {
        struct sockaddr_in6 addr;
        memset (&addr, 0, sizeof (addr));
        addr.sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
        addr.sin6_len = sizeof (addr);
#endif

        if (scope_id != 0
         && setsockopt (fd6, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                        &scope_id, sizeof (scope_id)))
        {
            perror("setsockopt(IPV6_MULTICAST_IF)");
            close (fd6);
            fd6 = -1;
            break;
        }
        
        if (i_ttl != 0)
            setsockopt (fd6, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                        &i_ttl, sizeof(i_ttl));

        if (bind (fd6, (struct sockaddr *)&addr, sizeof (addr)))
        {
            close (fd6);
            fd6 = -1;
            break;
        }
        shutdown (fd6, SHUT_RD);
        break;
    }

    /* Initializes IPv4 socket */
    fd4 = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    while (fd4 != -1)
    {
        struct sockaddr_in addr;
        memset (&addr, 0, sizeof (addr));
        addr.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
        addr.sin_len = sizeof (addr);
#endif

        if (psz_iface != NULL)
        {
            struct ifreq req;

            strncpy (req.ifr_name, psz_iface, IFNAMSIZ - 1);
            req.ifr_name[IFNAMSIZ - 1] = '\0';

            if (ioctl (fd4, SIOCGIFADDR, &req) < 0)
            {
                perror("ioctl(SIOCGIFADDR)");
                close (fd4);
                fd4 = -1;
                break;
            }
            
            struct in_addr *ip;
            ip = &((struct sockaddr_in *)(&req.ifr_addr))->sin_addr;
            if (setsockopt(fd4, IPPROTO_IP, IP_MULTICAST_IF,
                           (char *)ip, sizeof (*ip)))
            {
                perror("setsockopt(IP_MULTICAST_IF)");
                close (fd4);
                fd4 = -1;
                break;
            }
        }

        if (i_ttl && setsockopt (fd4, IPPROTO_IP, IP_MULTICAST_TTL,
                                 &i_ttl, sizeof(i_ttl)))
        {
            unsigned char ttl = i_ttl; /* Solaris wants a char */
            setsockopt (fd4, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, 1);
        }

        if (bind (fd4, (struct sockaddr *)&addr, sizeof (addr)))
        {
            close (fd4);
            fd4 = -1;
            break;
        }
        shutdown (fd4, SHUT_RD);
        break;
    }
}


Broadcast::~Broadcast()
{
    if (fd4 != -1)
        close (fd4);
    if (fd6 != -1)
        close (fd6);
}


int Broadcast::Send(Message* m, const struct sockaddr *dst, socklen_t len)
{
    /* Get the message and the length */
    const uint8_t* message = m->GetFinalMessage();
    size_t length = m->GetFinalMessageLen();

    if(message==NULL)
    {
        fprintf(stderr, "Bad message, skipping\n");
        return (-1);
    }

    int fd;
    switch (dst->sa_family)
    {
        case AF_INET6:
            fd = fd6;
            break;

        case AF_INET:
            fd = fd4;
            break;

        default:
            fd = -1;
    }

    if (fd == -1)
        return -1;

    if (sendto (fd, message, length, 0, dst, len) < 0)
    {
        perror ("sendto");
        return -1;
    }

    return 0;
}


int Broadcast::GuessDestination (const char *str,
                                 struct sockaddr_storage *addr,
                                 socklen_t *addrlen) const
{
    union
    {
        struct in6_addr in6;
        struct in_addr in;
    } n;

    if (inet_pton (AF_INET6, str, &n.in6) <= 0)
    {
        if (inet_pton (AF_INET, str, &n.in) <= 0)
        {
            fprintf (stderr, "%s: invalid IP address\n", str);
            return -1;
        }

        /* str is an IPv4 address */
        in_addr_t ip = ntohl (n.in.s_addr);

        // 224.0.0.0/24 => 224.0.0.255
        if ((ip & 0xffffff00) == 0xe0000000)
            ip =  0xe00000ff;
        else
        // 239.255.0.0/16 => 239.255.255.255
        if ((ip & 0xffff0000) == 0xefff0000)
            ip =  0xefffffff;
        else
        // 239.192.0.0/14 => 239.195.255.255
        if ((ip & 0xfffc0000) == 0xefc00000)
            ip =  0xefc3ffff;
        else
        // other multicast address => 224.2.127.254
        if ((ip & 0xf0000000) == 0xe0000000)
            ip = 0xe0027ffe;
        else
        {
            fprintf (stderr, "%s: not a multicast IPv4 address\n", str);
            return -1;
        }

        ip = htonl (ip);
        struct sockaddr_in *a4 = (struct sockaddr_in *)addr;
        memset (a4, 0, sizeof (*a4));
        a4->sin_family = AF_INET;
#ifdef HAVE_SA_LEN
        a4->sin_len = sizeof (*a4);
#endif
        a4->sin_port = htons (HELLO_PORT);
        memcpy (&a4->sin_addr.s_addr, &ip, sizeof (ip));
        *addrlen = sizeof (*a4);
        return 0;
    }

    /* str is an IPv6 address */
    if (n.in6.s6_addr[0] == 0xff)
    {
        /* multicast address */
        // ff0x::2:7ffe with x = scope
        n.in6.s6_addr[1] = n.in6.s6_addr[1] & 0xf;

        // (ff0x):0000:0000:0000:0000:0000:0002:7ffe
        memcpy (n.in6.s6_addr + 2,
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x7f\xfe",
                14);
    }
    else
    {
        fprintf (stderr, "%s: not a multicast IPv6 address\n", str);
        return -1;
    }

    struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)addr;
    memset (a6, 0, sizeof (*a6));
    a6->sin6_family = AF_INET6;
#ifdef HAVE_SA_LEN
    a6->sin_len = sizeof (*a6);
#endif
    a6->sin6_scope_id = scope_id;
    a6->sin6_port = htons (HELLO_PORT);
    memcpy (&a6->sin6_addr, &n.in6, sizeof (n.in6));
    *addrlen = sizeof (*a6);
    return 0;
}
