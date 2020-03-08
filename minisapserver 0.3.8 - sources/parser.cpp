/*****************************************************************************
 * parser.cpp : SAP configuration file parser
 ****************************************************************************
 * Copyright (C) 1998-2005 VideoLAN
 * $Id: parser.cpp 361 2010-10-09 12:14:24Z courmisch $
 *
 * Authors: Arnaud Schauly <gitan@via.ecp.fr>
 *          Clément Stenac <zorglub@via.ecp.fr>
 *          Damien Lucas <nitrox@videolan.org>
 *          Philippe Van Hecke <philippe.vanhecke@belnet.be>
 *          Derk-Jan Hartman <hartman at videolan dot org>
 *          Rémi Denis-Courmont <rem at videolan dot org>
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>

#ifdef HAVE_ICONV
# include <langinfo.h>
# include <iconv.h>
#else
//typedef unsigned iconv_t;
# define iconv_open( a, b ) ((iconv_t)(-1))
# define iconv_close( a ) ((void)0)
# define iconv( a, b, c, d, e ) ((size_t)(-1))
#endif


using namespace std;

#include "sapserver.h"
#include "program.h"
#include "message.h"
#include "parser.h"

Config::Config(string f) : daemon(false), dot(false)
{
    file       =    f;
    ttl        =    DEFAULT_TTL;
    interface  =    NULL;
    delay      =    DEFAULT_DELAY;
    type       =    TYPE_SAP;
    reverse    =    false;
}

Config::~Config()
{
    if (interface != NULL)
        free (interface);
    for (int i = Programs.size() - 1; i >= 0; i--)
        delete Programs[i];
}

unsigned int Config::GetTTL()
{
    return ttl;
}

void Config::SetTTL(unsigned int t)
{
    ttl=t;
}

unsigned int Config::GetDelay()
{
    return delay;
}

void Config::SetDelay(unsigned int d)
{
    delay=d;
}

bool Config::GetDaemonMode(void)
{
    return daemon;
}

bool Config::GetReverse(void)
{
    return reverse;
}

void Config::SetDaemonMode(bool d)
{
    daemon=d;
}

bool Config::GetDotMode(void)
{
    return dot;
}

void Config::SetDotMode(bool d)
{
    dot=d;
}


void Config::SetFile(char* s)
{
    file=s;
}

void Config::SetReverse(bool d)
{
    reverse=d;
}

void Config::SetType(const char *s)
{
    if(!strcasecmp(s,"slp"))
    {
	type=TYPE_SLP;  
    }
    else
    {
        type=TYPE_SAP;
    }
}

int Config::GetType()
{
    return type;
}

const char *Config::GetInterface()
{
    return interface;
}

/**********************************************************
 * Gets what is before delim in source. Puts it into dest *
 **********************************************************/
void Config::strgetb(const char *source,char *dest,char delim)
{
    char *ptr = dest;

    // Cut the line just before the delim char
    while(*source != '\0' && *source != delim)
        *(ptr++) = *(source++);
    *(ptr--) = '\0';

    // Remove the trailing blanks
    while((ptr >= dest) && isspace(*ptr))
        *(ptr--) = '\0';
}

/********************************************************** 
 * Gets what is after delim in source. Puts it into dest * 
 **********************************************************/ 
void Config::strgeta(const char *source,char *dest,char delim)
{
    unsigned long i=0,j=0;
    dest[0]=0;
    while(source[i] != delim && i < strlen(source))
    {
        i++;
    }
    i++;
    if(i>=strlen(source))
    {
        return;
    }
    while(i < strlen(source))
    {
        if(source[i]!='\n') dest[j]=source[i];
        else dest[j]=0;
        j++;i++;
    }
    dest[j]=0;
}


/********************************************************************
  Reads the configuration file and fills the program list
*********************************************************************/
int Config::Parse()
{
    FILE *fd;
    char line[1024];   //TODO line length should not be limited
    char tline[1024];
    bool something = false;
    Program *pp= new Program();
    iconv_t to_unicode, check_unicode;

    check_unicode = iconv_open("UTF-8", "UTF-8");
    to_unicode = iconv_open("UTF-8", nl_langinfo (CODESET));

    fd=fopen(file.c_str(),"r");
    if(!fd)
    {
        fprintf(stderr, "- Unable to open %s\n",file.c_str());
        return(-1);
    }

    while(fgets(line,1024,fd))
    {
        if(!strlen(line))
        {
            break;
        }

        strgetb(line,line,'#'); /* Handle the comments */

        /* Set SAP TTL  (ttl is a member of config) */
        if(strstr(line,"sap_ttl="))
        {
            strgeta(line,tline,'=');
            ttl=atoi(tline);
        }

        /* Set SAP Delay  (delay is a member of config) */
        if(strstr(line,"sap_delay="))
        {
            strgeta(line,tline,'=');
            delay=atoi(tline);
        }

        /* Set SAP Interface  (interface is a member of config) */
        if(strstr(line,"interface="))
        {
            strgeta(line,tline,'=');
            interface=strdup(tline);
        }

        /*  Beginning of programs parsing */
        if(strstr(line,"[program]"))
        {
            if(something)
            {
                /* We were in a program with at least one field filled */
                if (!pp->GetPort())
                {
                    pp->SetPort(DEFAULT_PORT);
                }
                Programs.push_back(pp);
                pp=new Program();
                something=false;
            }
        }

        if(strstr(line,"name="))
        {
            ICONV_CONST char *tptr;
            char *uptr, uline[6139];
            size_t tlen, ulen;

            strgeta(line,tline,'=');
            something=true;

            tptr=tline;
            uptr=uline;
            tlen=strlen(tline) + 1;
            ulen=sizeof(uline);
            if(iconv(check_unicode, &tptr, &tlen, &uptr, &ulen) == (size_t)(-1))
            {
                tptr=tline;
                uptr=uline;
                tlen=strlen(tline) + 1;
                ulen=sizeof(uline);
                if(iconv(to_unicode, &tptr, &tlen, &uptr, &ulen) == (size_t)(-1))
                    uptr = tline;
                else
                    uptr = uline;
            }
            else
                uptr = uline;

            pp->SetName(uptr);
        }

        if(strstr(line,"program_ttl="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetTTL(tline);
        }

        if(strstr(line,"playlist_group="))
        {
            ICONV_CONST char *tptr;
            char *uptr, uline[6139];
            size_t tlen, ulen;

            strgeta(line,tline,'=');
            something=true;

            tptr=tline;
            uptr=uline;
            tlen=strlen(tline) + 1;
            ulen=sizeof(uline);
            if(iconv(check_unicode, &tptr, &tlen, &uptr, &ulen) == (size_t)(-1))
            {
                tptr=tline;
                uptr=uline;
                tlen=strlen(tline) + 1;
                ulen=sizeof(uline);
                if(iconv(to_unicode, &tptr, &tlen, &uptr, &ulen) == (size_t)(-1))
                    uptr = tline;
                else
                    uptr = uline;
            }
            else
                uptr = uline;

            pp->SetPlGroup(uptr);
            pp->SetHasPlGroup(true);
        }

        if(strstr(line,"type=") && strstr(line,"rtp") )
        {
            pp->SetRTP(true);
        }

        if(strstr(line,"user="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetUser(tline);
        }

        if(strstr(line,"machine="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetMachine(tline);
        }

        if(strstr(line,"site="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetSite(tline);
        }

        if(strstr(line,"address="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetAddress(tline);
        }
        if(strstr(line,"port="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetPort(atoi(tline));
        }
        if(strstr(line,"customsdp="))
        {
            strgeta(line,tline,'=');
            something=true;
            pp->SetCustomSDP(tline);
        }
    }

    if(something)
    {
        if (!pp->GetPort())
        {
            pp->SetPort(DEFAULT_PORT);
        }
        Programs.push_back(pp);
    }
    else
        delete pp;

    fclose (fd);
    if(check_unicode != (iconv_t)(-1))
        iconv_close(check_unicode);
    if(to_unicode != (iconv_t)(-1))
        iconv_close(to_unicode);

    return(0);
}
