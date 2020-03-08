/*****************************************************************************
 * message.h : SAP Message Class definition
 ****************************************************************************
 * Copyright (C) 1998-2006 VideoLAN
 * $Id: message.h 325 2007-03-11 12:12:31Z courmisch $
 *
 * Authors: Damien Lucas <nitrox@videolan.org>
 *          Rémi Denis-Courmont <rem # videolan dot org>
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

#define SAP_ON 1
#define SAP_OFF 0
#define SAP_IPV4 4
#define SAP_IPV6 6

class Message {
  private:
    uint8_t* msg;         // final message
    size_t msg_len;
    uint16_t version;

  public:
    Message(uint16_t version, const char* ip);
    ~Message();
    bool AddProgram(Program*);
    uint8_t* GetFinalMessage(void) { return msg; }
    size_t GetFinalMessageLen(void) { return msg_len; }
};
