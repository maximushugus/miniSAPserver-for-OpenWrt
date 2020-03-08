/*****************************************************************************
 * program.h : SAP programs classes definition
 ****************************************************************************
 * Copyright (C) 1998-2002 VideoLAN
 * $Id: program.h 357 2009-04-25 20:44:33Z courmisch $ 
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


class Program
{
  public:
    Program(string, string, string, string, string, uint16_t);
    Program();
    ~Program();

    /*  Functions to get the values */
    string GetName();
    string GetUser();
    string GetMachine();
    string GetSite();
    string GetAddress();
    uint16_t GetPort();
    string GetTTL();
    string GetPlGroup();
    string GetCustomSDP();

    /* Functions to set the values */
    void SetName(const char*);
    void SetUser(const char*);
    void SetMachine(const char*);
    void SetSite(const char*);
    void SetAddress(const char*);
    void SetPort(uint16_t);
    void SetTTL(const char *);
    void SetPlGroup(const char *);
    void SetHasPlGroup(bool);
    void SetRTP(bool);
    void SetCustomSDP(const char *);

    bool IsPermanent();
    bool IsRTP();
    bool HasPlGroup();
    bool HasCustomSDP();

  private:
    string name;
    string user;
    string machine;
    string site;
    string address;
    string program_ttl;
    string pl_group;
    string custom_sdp;
    bool permanent;
    bool b_rtp;
    bool b_has_pl_group;
    uint16_t port;
    uint32_t start_time;
    uint32_t stop_time;
    /* TODO support for periodical programs */
};
