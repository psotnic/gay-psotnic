/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *   Copyright (C) 2009-2010 psotnic.com development team                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

penal::penal(int n)
{
	penalty = n;
	when = NOW;
}

penal::operator int()
{
	return (int) penalty;
}

void penal::update()
{
	int pen = penalty - NOW + when;

	when = NOW;
	penalty = pen > 0 ? pen : 0;
}

penal penal::operator+(int n)
{
	penal temp(penalty);

	temp.penalty += n;

	return temp;
}

penal penal::operator-(int n)
{
	penal temp(penalty);

	temp.penalty -= n;

	return temp;
}

penal penal::operator +=(int n)
{
	penalty += n;
	return *this;
}

penal penal::operator -=(int n)
{
	penalty -= n;
	return *this;
}

penal penal::operator++(int)
{
	++penalty;
	return *this;
}

penal penal::operator--(int)
{
	--penalty;
	return *this;
}

/** calculates penalty of a given command.
 *
 * \author patrick <patrick@psotnic.com>
 * \param data any string that should be sent to irc server
*/

void penal::calculatePenalty(const char *data)
{
    char argv[10][MAX_LEN], *name, *p=NULL;
    int len, argc, mypenalty=0;
    const int maxpenalty=10;

    len=strlen(data);
    mypenalty=(1+len/100);

    argc=str2words(argv[0], data, 10, MAX_LEN, 0);

    if(!strcasecmp(argv[0], "MODE"))
    {
        /* argv[1] = target; channels and/or user
         * argv[2] = optional modes
         * argv[n] = optional parameters
         */

        for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
        {
            if(chan::isChannel(name))
                calculatePenaltyOfChanmode(argv[2], argc-3, &mypenalty);

            else
                calculatePenaltyOfUsermode(argv[2], &mypenalty);
        }
    }

    else if(!strcasecmp(argv[0], "UMODE"))
    {
        /* argv[1] - username to change mode for
         * argv[2] - modes to change
         */
 
        calculatePenaltyOfUsermode(argv[2], &mypenalty);
    }

    else if(!strcasecmp(argv[0], "KICK"))
    {
        /* argv[1] = channel
         * argv[2] = client to kick
         * argv[3] = kick comment
         */

        int user_cnt=0;

        // count users to kick out
        for(name=strtok_r(argv[2], ",", &p); name; name=strtok_r(NULL, ",", &p))
            user_cnt++;

        // if there are multiple channels, the users will be kicked out on each one
        for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
            mypenalty+=3*user_cnt; // do not care if kick was successful, just to go the maximum

        /* alternative:
         * 
         * mypenalty+=user_cnt;
         * TODO: we must increase the penalty (+2) if the kick was successful -> parse_irc()
         */
    }

    else if(!strcasecmp(argv[0], "PRIVMSG") || !strcasecmp(argv[0], "NOTICE"))
    {
        // argv[1] = receiver list
        // argv[2] = text

        for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
            mypenalty+=1;
    }

    else if(!strcasecmp(argv[0], "TOPIC"))
    {
        /* argv[1] = channel list
         * argv[2] = topic
         */

        mypenalty+=1;

        if(*argv[2])
        {
            // changing topic
            for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
                mypenalty+=2;
        }
    }

    else if(!strcasecmp(argv[0], "AWAY"))
    {
        // argv[1] = away message

        if(!*argv[1]) // marking as not away
            mypenalty+=1;

        else // marking as away
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "MOTD"))
    {
        // argv[1] = servername

        if(*argv[1]) // remote MOTD
            mypenalty+=5;
        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "ADMIN"))
    {
        // argv[1] = servername

        if(*argv[1]) // remote ADMIN
            mypenalty+=3;

        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "INFO"))
    {
        // argv[1] = servername

        if(*argv[1]) // remote INFO
            mypenalty+=10;

        else
            mypenalty+=5;
    }

    else if(!strcasecmp(argv[0], "LINKS"))
    {
        /* argv[1] = servername mask
         * or:
         * argv[1] = server to query
         * argv[2] = servername mask
         */

        if(*argv[1] && *argv[2]) // remote LINKS
            mypenalty+=5;

        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "NAMES"))
    {
        /* argv[1] = channel list
         * argv[2] = server to query
         */
        
        if(*argv[2])
        {
            // query another irc server for NAMES
            mypenalty+=maxpenalty;
        }

        else if(*argv[1])
        {
            int chan_cnt=1;
            
            for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
                chan_cnt++;

            chan_cnt=chan_cnt<2?2:(chan_cnt*ME.server.isupport.maxchannels)/10;
            mypenalty+=chan_cnt<2?2:chan_cnt;
        }

        else
            mypenalty+=maxpenalty;
    }

    else if(!strcasecmp(argv[0], "LUSERS"))
    {
        /* argv[1] = host/server mask
         * argv[2] = server to query
         */

        if(*argv[1] && *argv[2]) // remote LUSERS
            mypenalty+=3;

        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "USERS"))
    {
        // argv[1] = servername

        if(*argv[1])  // remote USERS
            mypenalty+=3;

        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "WHO"))
    {
        // argv[1] = nickname mask or channel list
        // argv[2] = additional selection flag (like 'o')

        // FIXME: this can also be maxpenalty
        for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
            mypenalty+=1;
    } 

    else if(!strcasecmp(argv[0], "WHOIS"))
    {
        // argv[1] = nickname masklist

        mypenalty+=2;
        // XXX: we must increase penalty (+1) if we got a whois reply from another server
    }

    else if(!strcasecmp(argv[0], "WHOWAS"))
    {
        /* argv[1] = nickname
         * argv[2] = maximum replies
         * argv[3] = server to query
         */

        for(name=strtok_r(argv[1], ",", &p); name; name=strtok_r(NULL, ",", &p))
        {
            if(*argv[3])
                mypenalty+=3;

            else
                mypenalty+=2;
        }
    }

    else if(!strcasecmp(argv[0], "LIST"))
    {
        // argv[1] = channel list
        // argv[2] = server to query

        if(*argv[2]) // remote LIST
            mypenalty+=10;

        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "STATS"))
    {
	// usually STATS X remote server/nick
	if(*argv[2] && strcasecmp(argv[2], ME.nick))
	    mypenalty+=5; // maximum
        else
            mypenalty+=2;
    }

    else if(!strcasecmp(argv[0], "SQUERY"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "INVITE"))
        mypenalty+=3; // maximum

    else if(!strcasecmp(argv[0], "JOIN"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "PART"))
        mypenalty+=4;

    else if(!strcasecmp(argv[0], "NICK"))
    {
        if(net.irc.status & STATUS_REGISTERED)
            mypenalty+=3;
    }

    else if(!strcasecmp(argv[0], "TRACE"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "VERSION"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "SERVLIST"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "MAP"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "TIME"))
        mypenalty+=2;

    else if(!strcasecmp(argv[0], "HELP"))
        mypenalty+=2;

    else // everything else, e.g. PING, PONG, ISON
        mypenalty+=1;


    penalty+=mypenalty;
    DEBUG(printf("[D] Adding penalty %d (now: %d)\n", mypenalty-1, penalty));
}

/** calculates penalty of MODE command for channels.
 * Should only be executed by calculatePenalty()
 *
 * \author patrick <patrick@psotnic.com>
 * \param modes mode string like +ntk
 * \param argc number of arguments for the modes
 * \param mypenalty pointer to the penalty of calculatePenalty()
 */

void penal::calculatePenaltyOfChanmode(char *modes, int args, int *mypenalty)
{
    int idx, len;
    char sign='+';

    if(!modes || !*modes)
    {
        *mypenalty+=1;
        return;
    }

    len=strlen(modes);

    for(idx=0; idx<len; idx++)
    {
        if(modes[idx]=='+' || modes[idx]=='-')
        {
            sign=modes[idx];
            continue;
        }

        if(chan::chanModeRequiresArgument(sign, modes[idx]))
        {
            if(args<=0 && chan::getTypeOfChanMode(modes[idx])=='A') // list query
                *mypenalty+=1;

            else
                *mypenalty+=3;

            args--;
        }

        else
        {
            if(sign=='-' && modes[idx]=='l') // strange ..
                *mypenalty+=1;

            else
                *mypenalty+=3;
        }
    }
}

/** calculates penalty of MODE command for users.
 * Should only be executed by calculatePenalty()
 *
 * \author patrick <patrick@psotnic.com>
 * \param modes mode string like +iw
 * \param mypenalty pointer to the penalty of calculatePenalty()
 */

void penal::calculatePenaltyOfUsermode(char *modes, int *mypenalty)
{
    int idx, len;
    char sign='+';

    if(!modes || !*modes)
    {
        *mypenalty+=1;
        return;
    }

    len=strlen(modes);

    for(idx=0; idx<len; idx++)
    {
        if(modes[idx]=='+' || modes[idx]=='-')
        {
            sign=modes[idx];
            continue;
        }

        *mypenalty+=1;
    }
}
