/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
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
#include "defines.h"
#include "global-var.h"

/**
 * ent
 */
bool ent::operator<(const ent &e) const
{
	return strcmp(name, e.name) < 0 ? 1 : 0;
}

bool ent::operator==(const ent &e) const
{
	return !strcmp(name, e.name);
}

const char *ent::print(const int n) const
{
	static char buf[256];
	const char *v = getValue();
	snprintf(buf, 256, "%s %*s", name, n - strlen(name) + 3 + strlen(v), v);
	return buf;
}

const char *ent::getName() const
{
	return name;
}

options::event *ent::set(const char *arg, const bool justTest)
{
	return setValue(name, arg, justTest);
}

bool ent::isPrintable() const
{
	return dontPrintIfDefault ? !isDefault() : 1;
}

/**
 * entBool
 */
options::event ent::_event;

entBool::operator int() const
{
	return value;
}

const char *entBool::getValue() const
{
	return value ? "ON" : "OFF";
}

options::event *entBool::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		
		if(!arg2)
		{
		    _event.setError(this, "argument doesn't exists for entry %s", name); 
		    return &_event;
		}

		bool ok;
		bool n = str2int(arg2, ok);

		if(ok)
		{
			if(!justTest)
				value = n;

			_event.setOk(this, "%s has been turned %s", name, getValue());
			return &_event;
		}
		else
		{
			_event.setError(this, "argument is not a boolean value");
			return &_event;
		}
	}
	else if((*arg1 == '-' || *arg1 == '+') && !strcmp(arg1+1, name))
	{
		if(!justTest)
			value = (*arg1 == '+');

		_event.setOk(this, "%s has been turned %s", name, value ? "ON" : "OFF");
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

bool entBool::str2int(const char *str, bool &ok) const
{
	if(!strcmp(str, "yes") || !strcmp(str, "ON") ||
		   !strcmp(str, "enable") || !strcmp(str, "1"))
	{
		ok = true;
		return 1;
	}

	if(!strcmp(str, "no") || !strcmp(str, "off") ||
		   !strcmp(str, "disable") || !strcmp(str, "0"))
	{
		ok = true;
		return 0;
	}

	ok = false;
	return 0;
}

void entBool::reset()
{
	value = defaultValue;
}
bool entBool::isDefault() const
{
	return value == defaultValue;
}

/**
 * entInt
 */
options::event *entInt::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		bool ok;
		int n = str2int(arg2, ok);

		if(!ok)
		{
			_event.setError(this, "argument is not an integer type");
			return &_event;
		}
		if(n >= min && n <= max)
		{
			if(!justTest)
				value = n;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
		else
		{
			_event.setError(this, "argument does not belong to range <%s, %s>", getMin(), getMax());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

const char *entInt::getValue() const
{
	static char buf[20];
	sprintf(buf, "%d", value);
	return buf;
}

int entInt::str2int(const char *str, bool &ok) const
{
	ok = true;
	return atoi(str);
}

const char *entInt::getMin() const
{
	static char buf[20];
	sprintf(buf, "%d", min);
	return buf;
}

const char *entInt::getMax() const
{
	static char buf[20];
	if(max != MAX_INT)
		sprintf(buf, "%d", max);
	else
		strcpy(buf, "INFINITY");

	return buf;
}

entInt::operator int() const
{
	return value;
}

int entInt::operator==(int n) const
{
	return value == n;
}

/**
 * class entTime
 */
int entTime::str2int(const char *str, bool &ok) const
{
	int ret = 0;

	if(units2int(str, ut_time, ret) != 1)
	{
		ok = false;
		return 0;
	}

	ok = true;
	return ret;
}

const char *entTime::getValue() const
{
	static char buf[80];
	int2units(buf, 80, value, ut_time);

	return buf;
}

const char *entTime::getMin() const
{
	static char buf[80];
	int2units(buf, 80, min, ut_time);

	return buf;
}

const char *entTime::getMax() const
{
	static char buf[80];

	if(max != MAX_INT)
		int2units(buf, 80, max, ut_time);
	else
		strcpy(buf, "INFINITY");

	return buf;
}

/**
 * class entPerc
 */
int entPerc::str2int(const char *str, bool &ok) const
{
	int ret = 0;

	if(units2int(str, ut_perc, ret) != 1)
	{
		ok = false;
		return 0;
	}

	ok = true;
	return ret;
}

const char *entPerc::getValue() const
{
	static char buf[80];
	int2units(buf, 80, value, ut_perc);

	return buf;
}

const char *entPerc::getMax() const
{
	static char buf[80];
	int2units(buf, 80, min, ut_perc);

	return buf;
}

const char *entPerc::getMin() const
{
	static char buf[80];
	int2units(buf, 80, max, ut_perc);

	return buf;
}

/**
 * class entHost
 */
entHost::operator const char*() const
{
	return (const char *) connectionString;
}

const char *entHost::getValue() const
{
	return connectionString;
}

void entHost::reset()
{
#ifdef HAVE_ADNS
	resolve_pending = 0;
#endif
	ip = "0.0.0.0";
	connectionString = "0.0.0.0";
}

bool entHost::isDefault() const
{
	return !strcmp(connectionString, "0.0.0.0");
}

int entHost::getConnectionStringType(const char *str) const
{
	int type = 0;
	const char *ptr = str;
	bool search_prefix;

	do
	{
		search_prefix=false;

		if(!strncmp(ptr, "ssl:", 4))
		{
			type |= use_ssl;
			ptr += 4;
			search_prefix=true;
		}

		if(!strncmp(ptr, "ipv4:", 5))
		{
			type |= use_ipv4;
			ptr += 5;
			search_prefix=true;
		}

		if(!strncmp(ptr, "ipv6:", 5))
		{
			type |= use_ipv6;
			ptr += 5;
			search_prefix=true;
		}
	} while(search_prefix);

	switch(isValidIp(ptr))
	{
		case 4: 
			type |= ipv4;
			break;
		case 6:
			type |= ipv6;
			break;
		default:
			type |= domain;
	}

	return type;
}


options::event *entHost::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		int type = getConnectionStringType(arg2);
		int check = (type & (typesAllowed)) ^ (type &~ (use_ipv4 | use_ipv6));

		if(check != 0)
		{
			//TODO: add more verbous message here
			_event.setError(this, "%s is not valid: unsupported format", arg2);
			return &_event;
		}		

		const char *_arg = (type & use_ssl) ? arg2+4 : arg2;
		if(type & use_ipv4)
			_arg=_arg+5;
		if(type & use_ipv6)
			_arg=_arg+5;

		if(type & (ipv4 | ipv6))
		{
			if(!justTest)
			{
				ip = _arg;

				switch(isValidIp(_arg))
				{
					case 4 : ip4=_arg; break;
					case 6 : ip6=_arg; break;
				}

				connectionString = arg2;
			}		
			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
		else if(type & domain)
		{
			bool ok;
#ifdef HAVE_ADNS
			ok = true;
#else
			char buf[MAX_LEN];
			ok = false;
 #ifdef HAVE_IPV6
			if((typesAllowed & ipv6) && !(type & use_ipv4) && inet::gethostbyname(_arg, buf, AF_INET6))
				ok = true;
			else
 #endif
			if(!ok && !(type & use_ipv6) && (typesAllowed & ipv4) && inet::gethostbyname(_arg, buf, AF_INET))
				ok = true;
#endif
			if(ok)
			{
				if(!justTest)
				{
#ifdef HAVE_ADNS
					// dont resolve the hostname right here because:
					// 1) the config file is parsed before the resolver is initialized
					// 2) we probably dont need the ip now
					// -- patrick

					resolve_pending = 0;
#else
					ip = buf;
					switch(isValidIp(buf))
					{
						case 4 : if(!(type & use_ipv6))
							 	ip4=buf;
							break;
						case 6 : if(!(type & use_ipv4))
								ip6=buf;
							break;
					}
#endif
					connectionString = arg2;
				}

				_event.setOk(this, "%s has been set to %s", name, getValue());
			}
#ifndef HAVE_ADNS
			else if (errno)
				_event.setError(this, "Unknown host: %s (%s)", _arg, hstrerror(errno));
			else
				_event.setError(this, "Unknown host: %s", _arg);
#endif
			return &_event;
		}
		else
		{
			_event.setError(this, "FIXME: Unsupported type");
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

#ifdef HAVE_ADNS
/** Updates the ip address of an entHost entry.
 * \author patrick <patrick@psotnic.com>
 * \return 1 = resolved
 *         0 = waiting
 *        -1 = timed out
 */

int entHost::updateDnsEntry()
{
    int type=getConnectionStringType(connectionString);
    const char *_arg=(const char*) connectionString;
    adns::host2ip *info=NULL;

    if(type & (ipv4 | ipv6))
        return 1;

    // strip prefixes like 'ssl:'
    if(type & use_ssl)
        _arg=_arg+4;
    if(type & use_ipv4)
        _arg=_arg+5;
    if(type & use_ipv6)
        _arg=_arg+5;

    // check cache
    info=resolver->getIp(_arg);

    if(info) {
        if(typesAllowed & ipv6 && !(type & use_ipv4) && *info->ip6)
            ip6=info->ip6;

        if(typesAllowed & ipv4 && !(type & use_ipv6) && *info->ip4)
            ip4=info->ip4;

	// obsolete
        if(*ip6)
            ip=ip6;
        else if(*ip4)
            ip=ip4;

        resolve_pending=0;

	if(!*ip4 && !*ip6) {
            net.send(HAS_N, "Cannot resolve %s (%s)", _arg, (const char*) connectionString);    
            return -1;
        }

        return 1;
    }

    else {
        if(resolve_pending == 0) {
            resolver->resolv(_arg);
            resolve_pending=NOW;
            return 0;
        }

        if(NOW - resolve_pending > 60) {
            // timed out
            net.send(HAS_N, "Cannot resolve %s (%s) [timed out]", _arg, (const char*) connectionString);
            resolve_pending=0;
            return -1;
        }

	if(resolve_pending && !resolver->isResolving(_arg)) {
            // resolving failed
            net.send(HAS_N, "Cannot resolve %s (%s)", _arg, (const char*) connectionString);
            resolve_pending=0;
            return -1;
        }
    }

    return 0;
}
#endif
entHost::operator unsigned int() const
{
	return inet_addr(ip);
}

/**
 * class entString
 */
const char *entString::getValue() const
{
	return string;
}

options::event *entString::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		int n = strlen(arg2);

		if(n > max)
		{
			_event.setError(this, "argument is longer than %d characters", max);
			return &_event;
		}
		else if(n < min)
		{
			_event.setError(this, "argument is shorter than %d characters", min);
			return &_event;
		}
		else
		{
			if(!justTest)
				string = arg2;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

void entString::reset()
{
	string = defaultString;
}

bool entString::isDefault() const
{
	return !strcmp(defaultString, string);
}

entString::operator const char*() const
{
	return string;
}

int entString::getLen() const
{
	return string.len();
}

/**
 * class entWord
 */
options::event *entWord::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		if(countWords(arg2) > 1)
		{
			_event.setError(this, "argument is not a one word");
			return &_event;
		}

		int n = strlen(arg2);

		if(n > max)
		{
			_event.setError(this, "argument is longer than %d characters", max);
			return &_event;
		}
		else if(n < min)
		{
			_event.setError(this, "argument is shorter than %d characters", min);
			return &_event;
		}
		else
		{
			if(!justTest)
				string = arg2;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/**
 * class entHPPH
 */
options::event *entHPPH::_setValue(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		options::event *e;

		if(_host)
		{
			e = _host->set(arg2, 1);
			if(!e || !e->ok)
				return e;
		}
		if(_port)
		{
			e = _port->set(arg3, 1);
			if(!e || !e->ok)
				return e;
		}
		if(_pass)
		{
			e = _pass->set(arg4, 1);
			if(!e || !e->ok)
				return e;
		}

		if(_handle)
		{
			e = _handle->set(arg5, 1);
			if(!e || !e->ok)
				return e;
		}

		if(!justTest)
		{
			if(_host)
				_host->set(arg2);
			if(_port)
				_port->set(arg3);
			if(_pass)
				_pass->set(arg4);
			if(_handle)
				_handle->set(arg5);
		}

		_event.setOk(this, "%s has been set to %s", name, getValue());

		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

const char *entHPPH::getValue() const
{
	static char buf[1024];

	if(_host)
		strcpy(buf, _host->getValue());
	if(_port)
	{
		strcat(buf, " ");
		strcat(buf, _port->getValue());
	}
	if(_pass && *(_pass->getValue())!='\0')
	{
		strcat(buf, " ");
		strcat(buf, _pass->getValue());
	}
	if(_handle && *(_handle->getValue())!='\0')
	{
		strcat(buf, " ");
		strcat(buf, _handle->getValue());
	}

	return buf;
}

options::event *entHPPH::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	char arg[4][256];
	str2words(arg[0], arg2 ? arg2 : "", 4, 256);

	return _setValue(arg1, arg[0], arg[1], arg[2], arg[3], justTest);
}

void entHPPH::reset()
{
	if(_host)
		_host->reset();
	if(_port)
		_port->reset();
	if(_pass)
		_pass->reset();
	if(_handle)
		_handle->reset();
}

bool entHPPH::isDefault() const
{
	if(_host && !_host->isDefault())
		return false;
	if(_port && !_port->isDefault())
		return false;
	if(_pass && !_pass->isDefault())
		return false;
	if(_handle && !_handle->isDefault())
		return false;

	return true;
}

entHPPH::~entHPPH()
{
	if(_host)
		delete _host;
	if(_port)
		delete _port;
	if(_pass)
		delete _pass;
	if(_handle)
		delete _handle;
}

entHPPH &entHPPH::operator=(const entHPPH &e)
{
	name = e.name;
	dontPrintIfDefault = e.dontPrintIfDefault;
	readOnly = e.readOnly;

	if(_host)
		delete _host;
	if(_port)
		delete _port;
	if(_pass)
		delete _pass;
	if(_handle)
		delete _handle;

	if(e._host)
	{
		_host = new entHost();
		*_host = *e._host;
	}
	else
		_host = NULL;

	if(e._port)
	{
		_port = new entInt();
		*_port = *e._port;
	}
	else
		_port = NULL;

	if(e._pass)
	{
		if(typeid(*e._pass) == typeid(entWord))
			_pass = new entWord();
		else if(typeid(*e._pass) == typeid(entMD5Hash))
			_pass = new entMD5Hash();

		*_pass = *e._pass;
	}
	else
		_pass = NULL;

	if(e._handle)
	{
		_handle = new entWord();
		*_handle = *e._handle;
	}
	else
		_handle = NULL;

	return *this;
}

/**
 * class entMD5Hash
 */
options::event *entMD5Hash::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		//TODO: add [0-9a-f] check
		//its a hash
		int n = strlen(arg2);
		if(n == 32)
		{
			if(!justTest)
			{
				string =  arg2;
				quoteHex(arg2, hash);
			}
		}
		else
		{
			if(!justTest)
			{
				char buf[33];
				MD5Hash(hash, arg2, n);
				quoteHexStr(hash, buf, 16);
				string = buf;
			}
		}

		_event.setOk(this, "%s has been set to %s", arg1, getValue());
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

entMD5Hash::operator const unsigned char*() const
{
	return hash;
}

const unsigned char *entMD5Hash::getHash() const
{
	return hash;
}

void entMD5Hash::reset()
{
	memset(hash, 0, 16);
	string = "";
}

bool entMD5Hash::isDefault() const
{
	for(int i=0; i<16; ++i)
		if(hash[i])
			return 0;

	return 1;
}

/**
 * class entMult
 */
options::event *entMult::setValue(const char *arg1, const char *arg2, bool justTest)
{
	if(!strcmp(arg1, name) || (*arg1 == '+' && !strcmp(arg1+1, name)))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		ptrlist<ent>::iterator i = list.begin();
		while(i)
		{
			if(i->isDefault())
			{
				options::event *e = i->set(arg2, justTest);
				if(!e || !e->ok)
					return e;

				_event.setOk(i, "adding new entry: %s %s", i->name, i->getValue());
				return &_event;
			}
			i++;
		}
		_event.setError(this, "%s has reached maximum number of entries, please remove some entries in order to add new ones", name);
		return &_event;
	}
	else if(*arg1 == '-' && arg2 && *arg2 && !strcmp(arg1+1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		ptrlist<ent>::iterator i = list.begin();
		while(i)
		{
			if(!i->isDefault() && !strcmp(i->getValue(), arg2))
			{
				_event.setOk(i, "removing entry: %s %s", i->name, i->getValue());
				i->reset();
				return &_event;
			}
			i++;
		}
		_event.setError(this, "no such entry");
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

void entMult::reset()
{
	ptrlist<ent>::iterator i = list.begin();
	while(i)
	{
		i->reset();
		i++;
	}
}

void entMult::add(ent *e)
{
	list.addLast(e);
}

/**
 * entServer
 */
options::event *entServer::set(const char *ip, const char *port, const char *pass, const bool justTest)
{
	return _setValue(name, ip, port, pass, ip, justTest);
}

/**
 * entListener
 */
options::event *entListener::set(const char *_arg, const bool justTest)
{
        char arg[3][256];

	if(!_arg || !*_arg)
	{
		_event.setError(this, "+listen <ip> <port> <all|users|bots>", name);
		return &_event;
	}

        str2words(arg[0], _arg , 3, 256);

	if(*arg[2])
	{
		if(strcmp(arg[2], "all") && strcmp(arg[2], "users") && strcmp(arg[2], "bots"))
		{
		        _event.setError(this, "last argument must be \"all\", \"users\" or \"bots\"", name);
                        return &_event;
		}

		return _setValue(name, arg[0], arg[1], 0, arg[2], justTest);
	}

	else if(*arg[1])
		return _setValue(name, arg[0], arg[1], 0, "all", justTest);

	else
		return _setValue(name, "0.0.0.0", arg[0], 0, "all", justTest);
}


/**
 * entLoadModules
 */
//extern void registerAll(int (*_register)(const char *name, DLSYM_FUNCTION address));

bool entLoadModules::isDefault() const
{
	return *file=='\0';
}

void entLoadModules::reset()
{
	entLoadModules::unload(file);
	file="";
	md5sum="";
}

entLoadModules &entLoadModules::operator=(const entLoadModules &e)
{
	name=e.name;
	md5=e.md5;
	dontPrintIfDefault=e.dontPrintIfDefault;
	readOnly=e.readOnly;

	return *this;
}

const char *entLoadModules::getValue() const
{
	static char str[MAX_LEN];

	if(*md5sum)
		snprintf(str, MAX_LEN, "%s %s", (const char*)file, (const char*)md5sum); 
	else
		snprintf(str, MAX_LEN, "%s", (const char*)file);

	return str;
}

options::event *entLoadModules::setValue(const char *arg1, const char *arg2, bool justTest)
{
	char arg[2][256];
	str2words(arg[0], arg2 ? arg2 : "", 2, 256);

	return _setValue(arg1, arg[0], arg[1], justTest);
}

options::event *entLoadModules::_setValue(const char *arg1, const char *arg2, const char *arg3, bool justTest)
{
	if(!strcmp(arg1, name))
	{
#ifdef HAVE_MODULES
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		if(findModule(arg2))
		{
			_event.setError(this, "module is already loaded");
			return &_event;
		}

		int fd;
		unsigned char digest[16];
		char digestHex[33], path[MAX_LEN];

		if(*arg2 == '/')
			snprintf(path, MAX_LEN, "%s", arg2);
		else
			snprintf(path, MAX_LEN, "%s%s", MODULES_DIR, arg2);

		if(md5)
		{
			struct stat statbuf;

			// avoid crashes on systems that allow to open() a directory -- patrick

			if(stat(path, &statbuf) == 0)
			{
				if(!S_ISREG(statbuf.st_mode))
				{
					_event.setError(this, "cannot open %s: no regular file", path);
					return &_event;
				}
			}

			else
			{
				_event.setError(this, "cannot stat %s: %s", path, strerror(errno));
				return &_event;
			}

			fd = open(path, O_RDONLY);

			if(fd == -1)
			{
				_event.setError(this, "cannot open %s: %s", path, strerror(errno));
				close(fd);
				return &_event;
			}
			MD5HashFile(digest, fd);
			close(fd);

			quoteHexStr(digest, digestHex, 16);

			if(*arg3)
			{
				if(strcmp(digestHex, arg3))
				{
					_event.setError(this, "cannot load %s: md5 signature missmatch", path);
					return &_event;
				}
			}
		}
		else
			digestHex[0] = '\0';

		//int (*_register)(const char *name, DLSYM_FUNCTION address);
		module *(*init)();
		void *handle = dlopen(path, RTLD_LAZY);

		if(!handle)
		{
			_event.setError(this, "error while loading %s: %s", path, dlerror());
			return &_event;
		}

		/*_register = (int (*)(const char*, DLSYM_FUNCTION)) dlsym_cast(handle, "_register");
		if(!_register)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			return &_event;
		}*/

		init = (module*(*)()) dlsym_cast(handle, "init");
		if(!init)
		{
			_event.setError(this, "error while loading %s: %s", path, dlerror());
			return &_event;
		}

		//registerAll(_register);

		void *(*destroy)()=(void*(*)()) dlsym_cast(handle, "destroy");

		if(!destroy)
		{
			_event.setError(this, "error while loading %s: %s", path, dlerror());
			return &_event;
		}

		module *m = init();

		if(!justTest)
		{
			modules.addLast(m);
			m->file = arg2;
			m->md5sum = digestHex;
			m->loadDate = time(NULL);
			m->destroy=destroy;
			m->handle=handle;

			file=arg2;

			if(md5)
				md5sum=digestHex;
		}
		else
			dlclose(handle);

		_event.setOk(this, "loaded module %s", arg2);
		return &_event;

#else
	_event.setError(this, "This version of bot does not support modules");
	return &_event;
#endif
	}
	return NULL;
}

ptrlist<module>::iterator entLoadModules::findModule(const char *str)
{
	ptrlist<module>::iterator i = modules.begin();

	while(i)
	{
		if(!strcmp(i->file, str))
			return i;

			i++;
	}

	return i;
}


bool entLoadModules::unload(const char *str)
{
#ifdef HAVE_MODULES
	ptrlist<module>::iterator i = findModule(str);
	void *handle;

	if(i)
	{
		handle=i->handle;
		modules.removeLink(i);
		dlclose(handle);
		return true;
	}
#endif
	return false;
}


/*
bool entLoadModules::rehash(const char *str)
{
	if(
*/
//options::event *entLoadModules::_setValue(const char *arg1, const char *arg2, const char *arg3, bool justTest)
//moved to modules.cpp

