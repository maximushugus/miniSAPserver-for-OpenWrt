/*****************************************************************************
 * program.cpp : SAP Program class
 ****************************************************************************
 * Copyright (C) 1998-2002 VideoLAN
 * $Id: program.cpp 358 2009-04-25 20:45:09Z courmisch $
 *
 * Authors: Damien Lucas <nitrox@videolan.org>
 *          Philippe Van Hecke <philippe.vanhecke@belnet.be>
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

#include "program.h"

Program::Program() : b_rtp(false), b_has_pl_group(false)
{
    /* Set default Values */
    address="";
    port=0;
    permanent = true;
    program_ttl = "255";
    machine = "localhost";
    user = "VideoLAN";
    site = "http://www.videolan.org";
	custom_sdp = "";
}
Program::~Program() {return;}

string Program::GetName(void) {return name;}
string Program::GetUser(void) {return user;}
string Program::GetMachine(void) {return machine;}
string Program::GetSite(void) {return site;}
string Program::GetAddress(void){return address;}
uint16_t Program::GetPort(void){return port;}
string Program::GetTTL(void){return program_ttl;}
string Program::GetPlGroup(void){return pl_group;}
string Program::GetCustomSDP(void){return custom_sdp;}
bool   Program::IsPermanent(void){return permanent;}
bool   Program::IsRTP(void){return b_rtp;}
bool   Program::HasPlGroup(void){return b_has_pl_group;}
bool   Program::HasCustomSDP(void){return custom_sdp.size() ? true : false;}
void Program::SetName(const char* n){name=n;}
void Program::SetUser(const char* u){user=u;}
void Program::SetMachine(const char* m){machine=m;}
void Program::SetSite(const char* s){site=s;}
void Program::SetAddress(const char* a){address=a;}
void Program::SetPlGroup(const char *h){pl_group=h;}
void Program::SetRTP(bool b){b_rtp = b;}
void Program::SetHasPlGroup(bool b){b_has_pl_group = b ;}
void Program::SetPort(uint16_t p) { port = p; }
void Program::SetTTL(const char *p){program_ttl=p;}

Program::Program(string n, string u, string m, string s, string a,uint16_t p)
    : b_has_pl_group (false)
{
    name=n;
    user=u;
    machine=m;
    site=s;
    address=a;
    port=p;
    permanent=true;
}

void Program::SetCustomSDP(const char *fname)
{
    /* Although it's a (relatively small) fixed-sized buffer, we can't serve 
     * SDP any larger than this in any case - see the condition in
     * Message::AddProgram(): 
     *   if(msg_len + sdp.length () > 1024) - RFC 2974 Chap 6.
     */
    char buf[1024]; 
    FILE *f;
    ssize_t l;
    
    if(NULL == (f = fopen(fname, "rb")))
    {
        perror(fname);
        exit(1);
    }
    l = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[l] = '\0';
    custom_sdp = buf;
}
