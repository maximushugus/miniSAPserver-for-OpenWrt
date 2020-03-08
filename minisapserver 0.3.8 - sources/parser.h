/*****************************************************************************
 * parser.h : SAP configuration file parser definition
 ****************************************************************************
 * Copyright (C) 1998-2004 VideoLAN
 * $Id: parser.h 355 2009-03-04 15:09:19Z courmisch $ 
 *
 * Authors: Fabrice Ollivier <cif@via.ecp.fr>
 *          Arnaud Schauly <gitan@via.ecp.fr>
 *          Clément Stenac <zorglub@via.ecp.fr>
 *          Damien Lucas <nitrox@videolan.org>
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

class Config
{
  public:
    Config(string);
    ~Config();
    int Parse();
    unsigned int GetTTL();
    void SetTTL(unsigned int);
    unsigned int GetDelay();
    void SetDelay(unsigned int);
    bool GetDaemonMode(void);
    void SetDaemonMode(bool);
    bool GetReverse(void);
    void SetReverse(bool);
    void SetDotMode(bool);
    bool GetDotMode(void);
    void SetFile(char*);
    void SetType(const char*);
    int GetType();
    const char *GetInterface();
    vector<Program*> Programs;

  private:
    string file;
    int ttl;
    int delay;
    char * interface;
    int type;
    bool daemon;
    bool dot;
    bool reverse;
    void strgetb(const char *source,char *dest,char delim);
    void strgeta(const char *source,char *dest,char delim);
};
