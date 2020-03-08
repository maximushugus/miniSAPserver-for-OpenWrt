/*****************************************************************************
 * message.cpp : SAP Message class
 ****************************************************************************
 * Copyright (C) 1998-2006 the VideoLAN team
 * $Id: message.cpp 369 2012-03-14 15:19:59Z courmisch $
 *
 * Authors: Damien Lucas <nitrox@videolan.org>
 *          Philippe Van Hecke <philippe.vanhecke@belnet.be>
 *          Derk-Jan Hartman <hartman at videolan dot org>
 *          Rémi Denis-Courmont <rem # videolan dot org>
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

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "sapserver.h"
#include "program.h"
#include "message.h"

static const char mime_type[] = "application/sdp";

/*****************************************************************************
 * Constructors
 *****************************************************************************/
Message::Message(uint16_t v, const char* ip)
{
    version = v;

    in_addr_t ip_server = inet_addr (ip); //TODO automaticaly get it ?
    msg_len = 4
            + 4 /* (IPv4 specific) */
          /*+ authentification length (N/A) */
            + sizeof (mime_type);

    msg = (uint8_t *)realloc (NULL, msg_len);
    if (msg == NULL)
        return; // TODO: throw an exception

    // Build the Message Header once initialized according to RFC 2974 (SAP)

    /* Byte 0 : Version number V1        = 001      (3 bits)
     *          Address type   IPv4/IPv6 = 0/1      (1 bit)
     *          Reserved                   0        (1 bit)
     *          Message Type   ann/del   = 0/1      (1 bit)
     *          Encryption     on/off    = 0/1      (1 bit)
     *          Compressed     on/off    = 0/1      (1 bit) */
    msg[0] = 0x20;
    //if ( ip_version == SAP_IPV6 ) msg[0] |= 0x10;
    //if ( type == SAP_DELETE ) msg[0] |= 0x04;
    //if ( encrypted ) msg[0] |= 0x02;
    //if ( compressed ) msg[0] |= 0x01;

    /* Byte 1 : Authentification length - Not supported */
    msg[1] = 0x00;

    /* Bytes 2-3 : Message Id Hash */
    msg[2] = version & 0xff;
    msg[3] = version >> 8;

    /* Bytes 4-7 (or 4-19) byte: Originating source */
    memcpy (msg + 4, &ip_server, 4);

    /* finally MIME type */
    memcpy (msg + 8, mime_type, sizeof (mime_type));
}

/*****************************************************************************
 * Destructor
 *****************************************************************************/
Message::~Message(void)
{
    if (msg != NULL)
        free (msg);
}


bool Message::AddProgram(Program *p)
{
    string sdp = "v=0\r\n";  // SDP version
    /* FIXME */
    /* RFC 2327 Compliance ? */
    if (p->HasCustomSDP())
    {
        sdp = p->GetCustomSDP();
    }
    else
    {
        string ipv = (p->GetAddress().find(':') == string::npos) ? "IP4" : "IP6";

        string ver="";
        stringstream ssin(ver);
        ssin << version;
        ver = ssin.str();
        sdp += "o=" + p->GetUser() + " " + ver + " 1 IN " + ipv + " "
                  + p->GetMachine() + "\r\n";
        sdp += "s=" + p->GetName() + "\r\n";
        sdp += "u=" + p->GetSite() + "\r\n";

        sdp += "c=IN " + ipv + " " + p->GetAddress();
        if (ipv == "IP4")
            sdp += "/" + p->GetTTL(); /* only IPv4 multicast shall have a TTL */
        sdp += "\r\n";

        if (p->IsPermanent())
            sdp += "t=0 0\r\n";
        else
            return false;

        /* Session level attributes */
        sdp += "a=tool:" + (string)PACKAGE_STRING + "\r\n";

        if (p->HasPlGroup())
        {
            sdp += "a=cat:" + p->GetPlGroup() + "\r\n";
            sdp += "a=x-plgroup:" + p->GetPlGroup() + "\r\n";
        }

        /* Media and media-level attributes */
        sdp += "a=type:broadcast\r\n";
        sdp += "a=charset:UTF-8\r\n";

        char portbuf[6];
        snprintf(portbuf, sizeof(portbuf), "%u", (unsigned)p->GetPort());
        string port = portbuf;
        string m = "m=video " + port + " "
                 + (p->IsRTP() ? "RTP/AVP 33" : "udp mpeg") +"\r\n";

        if (p->IsRTP())
        {
            m += "a=rtpmap:33 MP2T/90000\r\n";
            if (p->IsRTP() & 1)
            {
               snprintf(portbuf, sizeof(portbuf), "%u", 1 + p->GetPort());
               port = portbuf;
               m += "a=rtcp:" + port + "\r\n";
            }
        }

        sdp += m;
    }
    puts (sdp.c_str ());

    if (msg_len + sdp.length () > 1024)
        //RFC 2974 Chap 6.
        return false;

    // updates full message
    uint8_t *newmsg = (uint8_t *)realloc (msg, msg_len + sdp.length ());
    if (newmsg == NULL)
        return false;

    // TODO: don't identically overwrite earlier programs
    memcpy (newmsg + msg_len, sdp.c_str(), sdp.length ());
    msg = newmsg;
    msg_len += sdp.length ();

    return true;
}
