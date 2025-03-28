/***************************************************************************
 *   Copyright (C) 2003-2007 by Grzegorz Rusin                             *
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
#include "global-var.h"
#include <sstream>

#define __itoa_num		16
char __itoa[16][16];

#define __timestr_num	16
char __timestr[256][16];

void tempCompatCheck(inetconn *c);

#ifdef HAVE_DEBUG
void doCryptoTests()
{
	int i;
	unsigned int size;
	unsigned char buf[16], out[16];
	
	srand(12345);
	printf("[D] Testing rand: ");
	for(i=0; i<8; ++i)
		printf("%X ", rand());
	printf("\n");

	printf("[D] Testing md5: ");
	MD5Hash(buf, "12345678", 8);
	for(i=0; i<16; i++)
		printf("%X ", buf[i]);
	printf("\n");

	printf("[D] Testing blowfish: ");
	CBlowFish blowfish;
	for(i=0; i<16; ++i)
		buf[i] = i*4;
	blowfish.Initialize(buf, 16);
	size = blowfish.Encode(buf, out, 16);
	for(i=0; i<(int)size; ++i)
		printf("%X ", out[i]);
	printf("\n");
}
#endif

void dumpIrcBacktrace()
{
#ifdef HAVE_IRC_BACKTRACE
	static char buf[256];
	snprintf(buf, 256, ".irc-backtrace-%d", (int) getpid());
	FILE *f = fopen(buf, "w");
	
	if(!f)
		return;
	
	int i;

	fprintf(f, "*** irc bufs: %d\n", IRC_BUFS);
	fprintf(f, "*** current buf: %d\n", current_irc_buf);
	
	for(i=0; i<IRC_BUFS; ++i)
		fprintf(f, "%02d: %s\n", i, irc_buf[i]);

	fprintf(f, "*** CRASH ***\n");
	fclose(f);
#endif
}

int isValidIp(const char *str)
{
	struct sockaddr_in sin;
#ifdef HAVE_IPV6
	struct sockaddr_in6 sin6;

	if(inet_pton(AF_INET6, str, (void *) &sin6.sin6_addr) > 0)
		return 6;
#endif
	if(inet_pton(AF_INET, str, (void *) &sin.sin_addr) > 0)
		return 4;

	return 0;
}

void hexDump(const char *str, int len)
{
	for(int i=0; i<len; ++i)
	{
		if(!(i % 16) && i)
		{
			printf("# ");
			for(int j=i-16; j<i; ++j)
			{
				if(isprint(str[j]))
					printf("%c", str[j]);
				else
					printf(".");
			}

			printf("\n");
		}

		printf("%02x ", abs(str[i]));
	}

	printf("\n");
}

void mem_strncpy(char *&dest, const char *src, int n)
{
	dest = (char *) malloc(n + 1);
	strncpy(dest, src, n);
	dest[n-1] = '\0';
}

void mem_strcpy(char *&dest, const char *src)
{
	if(src)
	{
		dest = (char *) malloc(strlen(src) + 1);
		strcpy(dest, src);
	}
	else
	{
		dest = (char *) malloc(1);
		*dest = '\0';
	}
}

void mem_strcat(char *&dest, const char *src)
{
	dest = (char *) realloc(dest, strlen(src) + strlen(dest) + 1);
	strcat(dest, src);
}

bool isNullString(const char *str, int len)
{
	int i;

	for(i=0; i<len; ++i)
		if(*str++) return false;
	return true;
}

int imUp()
{
	char pid[MAX_LEN];
	int fd;

	//read pid
	snprintf(pid, MAX_LEN, "pid.%s", (const char *) config.handle);
	if((fd = open(pid, O_RDONLY)) < 1)
		return 0;
	memset(pid, 0, MAX_LEN);
	if(read(fd, pid, MAX_LEN) < 1)
		return 0;

	close(fd);

	/*
	//read /proc/$pid/cmdline
	snprintf(tmp, MAX_LEN, "/proc/%d/cmdline", atoi(pid));
	if((fd = open(tmp, O_RDONLY)) < 1) return 0;
	memset(tmp, 0, MAX_LEN);
	n = read(fd, tmp, MAX_LEN);
	close(fd);

	for(i=0; i<n-1; ++i)
		if(!tmp[i]) tmp[i] = ' ';

	snprintf(buf, MAX_LEN, "*%s", config.file);

	return match(buf, tmp);
	*/

	if(atoi(pid) == getpid())
		return 0;

	// FIXME: some cronjob will send SIGHUP all time
	// which causes the bot to save the userlist etc
	// -- patrick
	return !kill(atoi(pid), SIGHUP);
}

#ifdef HAVE_ANTIPTRACE
void antiptrace_lurk()
{
	pid_t pid;
	int status;

	pid = fork();
	switch(pid)
	{
		case -1:
			printf("[-] Fork failed (antiptrace): %s\n", strerror(errno));
			exit(1);

		case 0:
			if(ptrace(PTRACE_TRACEME, 0, 0, 0) == -1)
			{
				printf("[-] Ptrace failed: %s\n", strerror(errno));
				exit(1);
			}
			break;

		default:
			printf("[+] Antiptrace loaded [pid: %d]\n", pid);
			while(1)
			{
				waitpid(pid, &status, 0);
				if(!WIFSTOPPED(status))
				{
					printf("[!] Antiptrace: child died, terminating\n");
					exit(0);
				}
				else
				{
					//printf("[CHILD] SIGNAL from child: %d\n", WSTOPSIG(status));
					ptrace(PTRACE_CONT, pid, status, 0, 0);
				}
			}
	}
}
#endif

void lurk()
{
	if(!config.dontfork)
	{
		pid_t pid = fork();

		if(pid == -1)
		{
			printf("[-] Fork failed: %s\n", strerror(errno));
			_exit(1);
		}
		else if(!pid)
		{
			printf("[+] Going into background [pid: %d]\n", (int) getpid());
			if(setsid() == -1)
				perror("[!] Cannot create new session: setsid()");
			freopen("/dev/null", "r", stdin);
			freopen("/dev/null", "w", stdout);
			freopen("/dev/null", "w", stderr);

			inetconn p;
			char buf[MAX_LEN];
			snprintf(buf, MAX_LEN, "pid.%s", (const char *) config.handle);
			p.open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			p.send("%d", getpid());
			return;
		}
		else
		{
			_exit(0);
		}
	}
}

int rmdirext(const char *dir)
{
#ifdef HAVE_DEBUG
	char buf[MAX_LEN];
#endif
	
	DEBUG(printf("[D] rmdir: cwd: %s\n", getcwd(buf, MAX_LEN)));
	if(chdir(dir))
		return -1;
	
	DEBUG(printf("[D] rmdir: cwd: %s\n", getcwd(buf, MAX_LEN)));
	
	struct stat s;
	DIR *d = opendir(".");
	struct dirent *ent;
	
	if(!d)
		return -1;
	
	DEBUG(printf("[D] rmdir: open dir: %p\n", (void *) d));
	while((ent = readdir(d)))
	{
		if(strcmp(".", ent->d_name) && strcmp("..", ent->d_name))
		{
			if(stat(ent->d_name, &s))
			{
				closedir(d);
				return -1;
			}
				
			if(S_ISDIR(s.st_mode))
			{
				if(rmdirext(ent->d_name))
				{
					closedir(d);
					return -1;
				}
			}
			else if(unlink(ent->d_name))
			{
				closedir(d);
				return -1;
			}
		}
	}

	closedir(d);

	DEBUG(printf("[D] rmdir: cwd: %s\n", getcwd(buf, MAX_LEN)));

	if(chdir("..") || rmdir(dir))
		return -1;
	
	return 0;
}

void parse_cmdline(int argc, char *argv[])
{
	int i;
#ifdef HAVE_DEBUG
	bool decrypted = false;
#endif

	if(argc == 1)
	{
#ifdef HAVE_DEBUG
		printf("Syntax: %s [-v] [-a] [-d] [-u] [-n] [-ne] [T decrypted config] [crypted config]\n", argv[0]);
#else
		printf("Syntax: %s [-v] [-a] [-n] [-ne] [crypted config]\n", argv[0]);
#endif
		exit(1);
	}

	propaganda();
	if(!strcmp(argv[1], "-v")) exit(0);

	for(i=1; i<argc; ++i)
	{
		if(!strcmp(argv[i], "-a"))
		{
			if(i+1 < argc)
				addToCron(i+1, argv, argc);
			else
				printf("No config files specified\n");
			exit(1);
		}

		if(!strcmp(argv[i], "-p"))
		{
			printf("Please use '%s -n' instead\n", argv[0]);
/*			char hash[33];
			printMessage("Bot is now running in MD5 hash generator mode");
			pstring<> toHash;
			readUserInput("String to hash", toHash);
			MD5HexHash(hash, toHash, toHash.len(), NULL, 0);
			printMessage("MD5 hash      : %s\n", hash);
*/			
			exit(0);
		}
#ifdef HAVE_DEBUG
		else if(!strcmp(argv[i], "-d")) debug = 1;
		/*else if(!strcmp(argv[i], "-u"))
		{
			psotget.doUpdate(argv[i] ? argv[i+1] : "");
			exit(0);
		}*/
		else if(!strcmp(argv[i], "--crypto-tests"))
		{
			doCryptoTests();
			exit(0);
		}
#endif

		else if(!strcmp(argv[i], "-v")) exit(0);

		else if(!strcmp(argv[i], "-c"))
		{
			printf("Please use '%s -n' instead\n", argv[0]);
			exit(0);
		}

		else if(!strcmp(argv[i], "-n"))
			createInitialConfig();

		else if(!strcmp(argv[i], "-ne"))
			createInitialConfig(true);

#ifdef HAVE_DEBUG
		else if(!strcmp(argv[i], "-T")) decrypted = true;
		else if(i == argc - 1) config.load(argv[i], decrypted);
#else
		else if(i == argc - 1) config.load(argv[i]);
#endif
		else
		{
			printf("Unknown option `%s'\n", argv[i]);
			exit(1);
		}
	}
}

/*! Prints out a formatted error message.
 * \param format The format string.
 * \param ... Variable list of paramaters.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void printError( const char *format, ... )
{
	char str[MAX_LEN] = "\033[1m\033[34m[\033[31m!\033[34m]\033[39m\033[22m \0";
	strncat( str, format, MAX_LEN-34 );
	strcat( str, "\n" );
	va_list va;
	va_start( va, format );
	vprintf( str, va );
	va_end( va );
}

/*! Prints out a formatted item string.
 * \param format The format string.
 * \param ... Variable list of parameters.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void printItem( const char *format, ... )
{
	char str[MAX_LEN] = "\033[1m\033[34m[\033[31m>\033[34m]\033[39m\033[22m \0";
	strncat( str, format, MAX_LEN-34 );
	strcat( str, "\n" );
	va_list va;
	va_start( va, format );
	vprintf( str, va );
	va_end( va );
}

/*! Prints out a formatted message.
 * \param format The format string.
 * \param ... Variable parameter list.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void printMessage( const char *format, ... )
{
	char str[MAX_LEN] = "\033[1m\033[34m[\033[33m*\033[34m]\033[39m\033[22m \0";
	strncat( str, format, MAX_LEN-34 );
	strcat( str, "\n" );
	va_list va;
	va_start( va, format );
	vprintf( str, va );
	va_end( va );
}

/*! Prints out a formatted prompt to stdout.
 * \param format The format string.
 * \param ...
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void printPrompt( const char *format, ... )
{
	char str[MAX_LEN] = "\033[1m\033[34m[\033[33m?\033[34m]\033[39m\033[22m \0";
	strncat( str, format, MAX_LEN-33 );
	va_list va;
	va_start( va, format );
	vprintf( str, va );
	va_end( va );
}

char *memmem(void *vsp, size_t len1, void *vpp, size_t len2)
{
	char *sp = (char *) vsp, *pp = (char *) vpp;
	char *eos   = sp + len1 - len2;

	if(!(sp && pp && len1 && len2))	return NULL;

	while (sp <= eos)
	{
		if (*sp == *pp)
		if (memcmp(sp, pp, len2) == 0) return sp;
		sp++;
	}
	return NULL;
}

long int nanotime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec;
}

char *srewind(const char *str, int word)
{
	int i;

	if(!str) return NULL;

	while(isspace(*str))
	{
		if(*str == '\0') return NULL;
		++str;
	}
	for(i=0; i<word; ++i)
	{
		while(!isspace(*str))
		{
			if(*str == '\0') return NULL;
			++str;
		}
		while(isspace(*str))
		{
			if(*str == '\0') return NULL;
			++str;
		}
	}
	return const_cast<char*> (str);
}

void str2args(char *word, const char *str, int x, int y)
{
	int i, j, c;

	for(i = 0; i < x; i++)
	{
		*word = '\0';
		while(*str && isspace(*str))
			str++;

		if(*str == '\0') break;

		for(j = 0, c = 0; (*str != '\0') && (j < (y - 1)) && 
				((!c && !isspace(*str)) || (c) )
				; str++)
		{
	    /*  --- zabezpieczenie przed obcinaniem slow po srodku ---

		jezeli slowo nie zaczyna sie od BEGIN_ARG_CHAR to znak BEGIN_CHAR_ARG 
		wystepujacy w tym slowie zostanie z ignorowany. podobnie dla znaku
		END_CHAR_ARG. jezeli znak za nim nie istnieje, lub nie jest to spacja to
		znaki END_CHAR_ARG w srodku sa ignorowane. 
		przyk�ady:
		str2args('cos "1 2"') = 'cos', '1 2'
	        str2args('cos 1" 2"') = 'cos', '1"', '2"'
		str2args('cos c"o"s" dupa') = 'cos', 'c"o"s"', 'dupa'
	    */
			if(!c && !j && *str == BEGIN_ARG_CHAR) 
			{
				c = 1;
				continue;	
			}

			if(c && *str == END_ARG_CHAR && (*(str+1) == '\0' || isspace(*(str+1))) )
				break;
			else
			{
				*(word++) = *str;
				j++;
			}
		}

		memset(word, 0, y - j - 1);
		word += y - j;

		if(c && *str == END_ARG_CHAR) str++;
		if(*str == '\0') break;
	}
	for(++i; i < x; i++)
	{
		memset(word, 0, y - 1);
		word += y;
	}
}

int str2words(char *word, const char *str, int x, int y, int ircstrip)
{
	int cnt=0, i, j, strip = 1;

	for(i=0; i<x; ++i)
	{
		*word = '\0';
		while(isspace(*str))
		{
			if(*str == '\0') break;
			++str;
		}
		if(*str == '\0') break;


		for(j=0; j<y-1 && !isspace(*str); ++str)
		{
			if(*str == '\0') break;
			if(ircstrip && !j && *str == ':' && strip) ;
			else
			{
				*word = *str;
				++word;
				++j;
			}
		}
		memset(word, 0, y - j - 1);

		if(ircstrip && i == 1 && (!strcmp(word-j, "MODE") || !strcmp(word-j, "JOIN")))
			strip = 0;

		word += y - j;
		cnt++;
		if(*str == '\0') break;
	}

	for(++i; i<x; ++i)
	{
		memset(word, 0, y - 1);
		word += y;
	}

	return cnt;
}

void sendLogo(inetconn *c)
{
	char *timestr;
	struct utsname name;

	timestr = ctime(&NOW);
	timestr[strlen(timestr) - 1] = '\0';

	uname(&name);

	c->send("");
	c->send("    _/_/_/   _/_/_/   _/_/_/  _/_/_/_/ _/      _/  _/   _/_/_/");
	c->send("   _/   _/ _/       _/    _/    _/    _/_/    _/  _/  _/");
	c->send("  _/_/_/   _/_/_/  _/    _/    _/    _/  _/  _/  _/  _/");
	c->send(" _/            _/ _/    _/    _/    _/    _/_/  _/  _/");
	c->send("_/       _/_/_/   _/_/_/     _/    _/      _/  _/   _/_/_/");
	c->send("");
	c->send("   Copyright (c) 2003-2007 Grzegorz Rusin <grusin@gmail.com>");
	c->send("   Copyright (C) 2009-2010 psotnic.com development team");
	c->send("");

	if(c->checkFlag(HAS_N))
	{
		unsigned int i = userlist.offences();
		
		c->send("%s version: %s", S_BOTNAME, S_VERSION);
		c->send("Local time: %s", timestr);
		c->send("Machine info: %s %s %s", name.sysname, name.release, name.machine);
		c->send("Owners on-line: %d (type .owners to see list)", net.owners());
		c->send("Bots on-line: %d (type .upbots to see list)", net.bots() + 1);
		if(i)
		    c->send("New offences: %d (type .offences to see list)", i);
		c->send("");
		net.send(HAS_N, "%s has joined the partyline", c->name);
		ME.checkMyHost(c->name);
		net.propagate(NULL, "%s %s", S_CHKHOST, c->name);
		tempCompatCheck(c);

		if(updateNotify)
			net.sendOwner(c->name, "I have been updated and need to be restarted");

	}
	else if(c->checkFlag(HAS_P))
	{
		net.send(HAS_P, "%s has joined the partyline", c->name);
	}
	else
	{
		net.send(HAS_N, "\002%s has hacked into the partyline\002", c->name);
		c->close("Fuckin' hacka");
	}
}

#ifdef HAVE_IPV6
int doConnect6(const char *server, int port, const char *vhost, int noblock)
{
	struct sockaddr_in6 sin6;
	int s;

	s = socket(AF_INET6, SOCK_STREAM, 0);
	if(s == -1)
		return -1;

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	if(vhost && *vhost)
		inet_pton(AF_INET6, vhost, (void *) &sin6.sin6_addr);
	else
		sin6.sin6_addr = in6addr_any;

	if(bind(s, (struct sockaddr *) &sin6, sizeof (sin6)) == -1)
	{
		net.send(HAS_N, "Cannot bind() to %s: %s", (vhost && *vhost) ? vhost : "in6addr_any", strerror(errno));
		killSocket(s);
		return -1;
	}

	memset(&sin6, 0, sizeof (struct sockaddr_in6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = htons(port);
	inet_pton(AF_INET6, server, (void *) &sin6.sin6_addr);

	if(noblock == -1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
	{
		net.send(HAS_N, "fcntl() failed: %s", strerror(errno));
		killSocket(s);
		return -1;
	}
	
	if(connect(s, (struct sockaddr *) &sin6, sizeof(sin6)) == -1)
	{
		if(noblock == -1 && errno == EINPROGRESS)
			return s;
		killSocket(s);
		return -1;
	}
	
	if(noblock == 1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
	{
		killSocket(s);
		return -1;
	}
	return s;
}
#endif

void propaganda()
{
	printf("\n");
#ifdef HAVE_IPV6
	printf("%s, version %s-ipv6 (rev: %s, build: %s %s)", S_BOTNAME, S_VERSION, SVN_REVISION, __DATE__, __TIME__);
#else
	printf("%s, version %s (rev: %s build: %s %s)", S_BOTNAME, S_VERSION, SVN_REVISION, __DATE__, __TIME__);
#endif
	printf("\n");
	printf("Copyright (C) 2003-2007 Grzegorz Rusin <grusin@gmail.com>\n");
	printf("Copyright (C) 2009 psotnic.com development team\n");
	printf("\n");
}

bool extendhost(const char *host, char *buf, unsigned int len)
{
	char *ex, *at;

	if(strlen(host) + 10 > len || !isRealStr(host) || *host == '#')
		return false;

	ex = strchr((char *)host, '!');
	at = strchr((char *)host, '@');

	if(ex != strrchr((char *)host, '!') || at != strrchr((char *)host, '@'))
		return false;
	
	if(at)
	{
		if(!ex)
		{
			if(at == host) strcpy(buf, "*!*");
			else strcpy(buf, "*!");
			strcat(buf, host);
		}
		else if(ex == host)
		{
			strcpy(buf, "*");
			strcat(buf, host);
		}
		else strcpy(buf, host);
		if(*(at + 1) == '\0') strcat(buf, "*");
		return true;
	}
	else
	{
		if(ex) return false;
		if(strchr((char *)host, '.') || strchr((char *)host, ':'))
		{
			strcpy(buf, "*!*@");
			strcat(buf, host);
			return true;
		}
		strcpy(buf, "*!");
		strcat(buf, host);
		strcat(buf, "@*");
		return true;
	}
}

/** Generates new nicknames.
 * This function will be executed when the bot's nickname is already in use during registration.
 * At first it tries to append chars of config.nickappend in all variations.
 * After that it will append numbers at the 9th position.
 *
 * nicklen variable of class server cannot be used because the bot is not connected yet.
 * Do not increase 9 to 15.
 *
 * \author patrick <patrick@psotnic.com>
 */

void nickCreator(char *nick)
{
	int i, nicklen=strlen(nick), applen=strlen(config.nickappend);
	char *n;

	if(nicklen<9 || (nicklen==9 && strchr(config.nickappend, nick[8])))
	{
		for(i=nicklen-1; i>=0; i--)
		{
			if(i!=0)
			{
				if(nick[i]==config.nickappend[applen-1])
				{
					nick[i]=config.nickappend[0];
					continue;
				}

				if((n=strchr((char *)((const char *)config.nickappend), nick[i])))
				{
					n++;
					nick[i]=*n;
					return;
				}
			}

			if(nicklen>=9)
				break;

			nick[nicklen]=config.nickappend[0];
			nick[nicklen+1]='\0';
			return;
		}
	}

	for(i=8; i>0; i--)
	{
		if(nick[i]=='9')
		{
			nick[i]='0';
			continue;
		}

		if(nick[i]>='0' && nick[i]<'9')
		{
			nick[i]++;
			return;
		}

		nick[i]='1';
		return;
	}
}

char *inet2char(unsigned int inetip)
{
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = inetip;
	return inet_ntoa(sin.sin_addr);
}

#ifdef HAVE_IPV6
static char _inet6charbuf[64];
const char *inet6char(in6_addr *addr)
{
	return inet_ntop(AF_INET6, addr, _inet6charbuf, 64);
}
#endif

int acceptConnection(int fd, bool ssl, inet::listen_entry *le)
{
	int n, silent;
	struct sockaddr_in from;
	socklen_t fromsize = sizeof(struct sockaddr_in);
	const int one = 1;
	inetconn *c;

	if((n = accept(fd, (sockaddr *) &from, &fromsize)) > 0)
	{
		ign::entry *e = ignore.hit(ntohl(from.sin_addr.s_addr));

		if(e->nextConn >= NOW || ignore.nextConn >= NOW)
		{
			killSocket(n);
			return -1;
		}

		silent = e->count > set.PERIP_MAX_SHOWN_CONNS;

		//if(userlist.isSlave(userlist.first->next) || userlist.isMain(userlist.first->next) || config.bottype == BOT_SLAVE || config.bottype == BOT_MAIN)
		if(1)
		{
			if((le->access & (LISTEN_ALL | LISTEN_USERS))
				|| ((le->access & LISTEN_BOTS) && (userlist.isBot(from.sin_addr.s_addr) || userlist.isBotByFD(fd))))
			{
				fcntl(n, F_SETFL, O_NONBLOCK);
				setsockopt(n, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
				c = net.addConn(n);
				c->tmpint = 1;
				c->killTime = NOW + set.AUTH_TIMEOUT;
				c->status = STATUS_CONNECTED + STATUS_BOT + STATUS_TELNET + (silent ? STATUS_SILENT : 0);
				c->listener_opt=le->access;
				
#ifdef HAVE_SSL
				if(ssl)
				{
					bool ssl_error=false;

					//c->status &= ~STATUS_CONNECTED;
					c->ssl_ctx = NULL;
					c->status |= STATUS_SSL;
					c->ssl = SSL_new(inet::server_ctx);
					SSL_set_fd(c->ssl, c->fd);
					
					int ret = SSL_accept(c->ssl);
					
					DEBUG(printf("[D] SSL: accept: %d\n", ret));

					switch(ret)
					{
						case 0:
						c->close("Handshake terminated");
						DEBUG(printf("[D] SSL: Handshake terminated\n"));
						ssl_error=true;
						break;
						
						case -1:
						switch(SSL_get_error(c->ssl, ret))
						{
							case SSL_ERROR_WANT_READ:
							case SSL_ERROR_WANT_WRITE:
							//c->status |= STATUS_SSL_HANDSHAKING | STATUS_SSL_WANT_ACCEPT;
							DEBUG(printf("[D] SSL: want accept\n"));
							break;
							
							default:
							c->close("SSL handshake failed");
							DEBUG(printf("[D] SSL: handshake failed\n"));
							ssl_error=true;
							break;
						}
						break;
						
						default:
						c->status |= STATUS_CONNECTED;
						DEBUG(printf("[D] SSL socket is connected!!!\n"));
						break;
					}

					if(ssl_error)
						return -1;
				}
#endif

				if(!silent)
				{
					net.send(HAS_N, "Accepting connection from %s / %s to %s / %s",
                             c->getPeerIpName(), c->getPeerPortName(), c->getMyIpName(), c->getMyPortName());
				}

				return n;
			}
			else
			{
				c = new inetconn;
				c->fd = n;

				if(!silent)
				{
					net.send(HAS_N, "\002Rejecting connection from %s / %s to %s / %s\002",
                             c->getPeerIpName(), c->getPeerPortName(), c->getMyIpName(), c->getMyPortName());
				}

				DEBUG(printf("[!] Rejecting connection: %s / %s", c->getPeerIpName(), c->getPeerPortName()));
				killSocket(n);
				delete c;
				return -1;
			}
		}
		else
		{
			killSocket(n);
			return -1;
		}
	}
	else
		return -1;
}

const char *getipstr(int fd, int proto, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen))
{

	if(proto == AF_INET)
	{
		struct sockaddr_in peer;
		#ifdef _NO_LAME_ERRNO
		int ret, e = errno;
		#endif
		socklen_t peersize = sizeof(struct sockaddr_in);
		ret = fun(fd, (sockaddr *) &peer, &peersize);
		#ifdef _NO_LAME_ERRNO
		errno = e;
		#endif
		if(ret == -1) return NULL;
		else return inet_ntoa(peer.sin_addr);
	}
#ifdef HAVE_IPV6
	else
	if(proto == AF_INET6)
	{
		struct sockaddr_in6 peer;
		#ifdef _NO_LAME_ERRNO
		int ret, e = errno;
		#endif
		socklen_t peersize = sizeof(struct sockaddr_in6);
		ret = fun(fd, (sockaddr *) &peer, &peersize);
		#ifdef _NO_LAME_ERRNO
		errno = e;
		#endif
		if(ret == -1) return NULL;
		else return inet6char(&peer.sin6_addr);
	}
#endif
	return NULL;
}

unsigned int getip4(int fd, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen))
{
	struct sockaddr_in peer;
	#ifdef _NO_LAME_ERRNO
	int ret, e = errno;
	#endif
	socklen_t peersize = sizeof(struct sockaddr_in);
	ret = fun(fd, (sockaddr *) &peer, &peersize);
	#ifdef _NO_LAME_ERRNO
	errno = e;
	#endif
	if(ret == -1) return 0;
	else return (unsigned int) inet_netof(peer.sin_addr);
}

int getport(int fd, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen))
{
	struct sockaddr_in peer;
	#ifdef _NO_LAME_ERRNO
	int ret, e = errno;
	#endif
	socklen_t peersize = sizeof(struct sockaddr_in);
	ret = fun(fd, (sockaddr *) &peer, &peersize);
	#ifdef _NO_LAME_ERRNO
	errno = e;
	#endif
	if(ret == -1) return 0;
	else return ntohs(peer.sin_port);
}

int startListening(const char *ip, int port)
{
	struct sockaddr_in sin;
#ifdef HAVE_IPV6
        struct sockaddr_in6 sin6;
#endif
	int s;
	const int one = 1;
	int proto;
	int ret;

	if(!ip || !*ip)
		return -1;
#ifdef HAVE_IPV6
	proto=isValidIp(ip);
#else
	proto=4;
#endif
	if((s = socket(proto == 6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0)) == -1)
	{
		killSocket(s);
		return -1;
	}

	if(setsockopt(s , SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0)
	{
		killSocket(s);
		return -1;
	}
#ifdef HAVE_IPV6
	if(proto == 6)
	{
		memset (&sin6, 0, sizeof (struct sockaddr_in6));
		sin6.sin6_family = AF_INET6;
		inet_pton(AF_INET6, ip, (void *) &sin6.sin6_addr);
		sin6.sin6_port = htons(port);
		ret=bind (s, (struct sockaddr *) &sin6, sizeof (struct sockaddr_in6));
	}

	else
	{
#endif
		memset (&sin, 0, sizeof (struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(ip);
		sin.sin_port = htons(port);
		ret=bind (s, (struct sockaddr *) &sin, sizeof (struct sockaddr_in));
#ifdef HAVE_IPV6
	}
#endif
	if(ret == -1)
	{
		killSocket(s);
		return -1;
	}

	if(listen(s, SOMAXCONN) == -1)
	{
		killSocket(s);
		return -1;
	}

	return s;
}

void precache()
{
	srand();
	//validate();

#ifdef HAVE_DEBUG
	debug = 0;
#endif
	config.currentServer = NULL;
#ifdef HAVE_IRC_BACKTRACE
	for(int i=0; i<IRC_BUFS; ++i)
		*irc_buf[i] = 0;
#endif

	precache_expand();

#ifdef HAVE_MODULES
	modules.removePtrs();
#endif

#ifdef HAVE_SSL
	ERR_load_crypto_strings();
	ERR_load_SSL_strings();
	OpenSSL_add_all_algorithms();
#endif
}

void precache_expand()
{
	struct utsname name;
	uname(&name);

	expandinfo.system = name.sysname;
	expandinfo.release = name.release;
	expandinfo.arch = name.machine;

	char buf[MAX_LEN];
	switch(config.ctcptype)
	{
		case VER_PSOTNIC: expandinfo.version = S_VERSION; break;
		case VER_IRSSI: expandinfo.version = fetchVersion(buf, VER_IRSSI); break;
		case VER_EPIC:
		case VER_LICE: expandinfo.version = fetchVersion(buf, VER_EPIC); break;
		case VER_BITCHX: expandinfo.version = fetchVersion(buf, VER_BITCHX); break;
		default: expandinfo.version = ""; break;
	}

	//struct passwd *p = getpwuid(getuid());
	expandinfo.realname = "";

}

void divide(int *ret, int value, int parts, int part_size)
{
	if(parts == 1)
	{
		ret[0] = value;
		ret[1] = ret[2] = 0;
		return;
	}

	if(value > part_size*2)
	{
		ret[0] = value / parts;
		ret[1] = (value - ret[0]) / (parts-1);
		if(parts == 3) ret[2] = value - ret[0] - ret[1];
		else ret[2] = 0;
	}
	else if(value > part_size)
	{
		ret[0] = part_size;
		ret[1] = value - ret[0];
		ret[2] = 0;
	}
	else
	{
		ret[0] = value;
		ret[1] = ret[2] = 0;
	}
}

void killSocket(int fd)
{
	#ifdef _NO_LAME_ERRNO
	int e = errno;
	#endif
	shutdown(fd, SHUT_RDWR);
	close(fd);
	#ifdef _NO_LAME_ERRNO
	errno = e;
	#endif
}

int doConnect(const char *server, int port, const char *vhost, int noblock)
{
	struct sockaddr_in sin;
	int s;
	unsigned int server_addr, vhost_addr;

	server_addr=inet_addr(server);

	if(vhost && *vhost)
		vhost_addr=inet_addr(vhost);
	else
		vhost_addr=0;

	s = socket(AF_INET, SOCK_STREAM, 0);

	if(s == -1)
		return -1;

	memset (&sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;

	if(vhost_addr)
		sin.sin_addr.s_addr = vhost_addr;

	else
		sin.sin_addr.s_addr = INADDR_ANY;

	if(bind (s, (struct sockaddr *) &sin, sizeof (sin)) == -1)
	{
		net.send(HAS_N, "Cannot bind() to %s: %s", vhost_addr ? vhost : "INADDR_ANY", strerror(errno));
		killSocket(s);
		return -1;
	}

	memset (&sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = server_addr;

	if(noblock == -1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
	{
		net.send(HAS_N, "fcntl() failed: %s", strerror(errno));
		killSocket(s);
		return -1;
	}
	if(connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	{
		if(noblock == -1 && errno == EINPROGRESS)
			return s;

		// FIXME: this does not work somehow.
		// when connecting to localhost the bot sends NICK/USER
		// even when connection fails (same with doConnect6()) -- patrick

		killSocket(s);
		return -1;
	}
	if(noblock == 1 && fcntl(s, F_SETFL, O_NONBLOCK) == -1)
	{
		killSocket(s);
		return -1;
	}
	return s;
}

unsigned int hash32(const char *word)
{
	if(!word) return 0;
	char c;
	unsigned bit_hi = 0;
	int bit_low = 0;
	int len = strlen(word);

	if(len > 64) len = 63;

	/* i barrowed this code from epic ;-) */
	for(; *word && len; ++word, --len)
	{
		c = ircd_tolower(*word);
		bit_hi = (bit_hi << 1) + c;
		bit_low = (bit_low >> 1) + c;
	}
	return ((bit_hi & 8191) << 3) + (bit_low & 0x7);
}

int va_getlen(va_list ap, const char *lst)
{
	int size = strlen(lst);
	char *p;

	while((p = va_arg(ap, char *)))
		size += strlen(p);

	return size;
}

char *push(char *ptr, const char *lst, ...)
{
	va_list ap;
	int len;

	va_start(ap, lst);
	len = va_getlen(ap, lst);
	va_end(ap);

	va_start(ap, lst);
	ptr = va_push(ptr, ap, lst, len + 1);
	va_end(ap);

	return ptr;
}

char *va_push(char *ptr, va_list ap, const char *lst, int size)
{
	char *p;

	if(ptr)
	{
		size += strlen(ptr);
		ptr = (char *) realloc(ptr, size*sizeof(char));
		strcat(ptr, lst);
	}
	else
	{
		ptr = (char *) malloc(size*sizeof(char));
		strcpy(ptr, lst);
	}

	/* strcat rest */
	while((p = va_arg(ap, char *)))
	{
		strcat(ptr, p);
	}
	return ptr;
}


char *itoa(int value)
{
	static int i=0;

	i %= __itoa_num;
	sprintf(__itoa[i], "%d", value);
	return __itoa[i++];
}

char *expand(const char *str, char *buf, int len, const char *args)
{
	int i, j;

	memset(buf, 0, len--);

	for(i=j=0; ; ++i, ++str)
	{
		if(*str == '\0') break;
		if(*str == '%')
		{
			++str;
			if(*str == '\0') break;
			switch(*str)
			{
				case 'V': strncat(buf, expandinfo.version, len - j); j += strlen(expandinfo.version); break;
				case 'O': strncat(buf, expandinfo.system, len - j); j += strlen(expandinfo.system); break;
				case 'R': strncat(buf, expandinfo.release, len - j); j += strlen(expandinfo.release); break;
				case 'A': strncat(buf, expandinfo.arch, len - j); j += strlen(expandinfo.arch); break;
				case 'N': strncat(buf, ME.nick, len - j); j += strlen(ME.nick); break;
				case 'I': strncat(buf, ME.ident, len - j); j += strlen(ME.ident); break;
				case 'H': strncat(buf, ME.host, len - j); j += strlen(ME.host); break;
				case '*': strncat(buf, args, len - j); j += strlen(args); break;
				case 'F': strncat(buf, expandinfo.realname, len - j); j += strlen(expandinfo.realname); break;
				case 'T':
				{
					char *t = ctime(&NOW);
					t[strlen(t) - 1] = '\0';
					strncat(buf, t, len - j); j += strlen(t);
					break;
				}
				/*case 'l':
				{
					int t = antiidle.getIdle();
					strncat(buf, itoa(t), len - j); j += strlen(itoa(t));
					break;
				}*/
				default: break;
			}
		}
		else
		{
			if(j++ < len - 1) strncat(buf, str, 1);
			else return buf;
		}
	}
	return buf;
}

const char *getFileName(const char *path)
{
	const char *last = path;

	while(*path)
	{
		if(*path == '/') last = path + 1;
		++path;
	}
	return last;
}

int sign(int n)
{
	if(!n) return 0;
	if(n > 0) return 1;
	return -1;
}

char *int2units(char *buf, int len, int val, unit_table *ut)
{
	char tmp[32];
	int i, k, v = val;

	if(!ut)
	{
		plain:
		strncpy(buf, itoa(val), MAX_LEN);
		return buf;
	}
	else if(!val)
	{
		for(i=0; ut[i].unit; ++i) ;

		sprintf(buf, "0%c", ut[i-1].unit);
		//strncpy(buf, itoa(val), MAX_LEN);
		//tmp[0] = ut[i-1].unit;
		//tmp[1] = '\0';
		//strcat(buf, tmp);
		return buf;
	}

	buf[0] = '\0';

	for(i=0; ut[i].unit; i++)
	{
		if(sign(val) == sign(ut[i].ratio) && (k = v/ut[i].ratio))
		{
			strcpy(tmp, itoa(k));
			len -= strlen(tmp) + 3;
			if(len <= 0) return NULL;
			v -= k * ut[i].ratio;
			strcat(buf, tmp);
			tmp[0] = ut[i].unit;
			if(v)
			{
				tmp[1] = ' ';
				tmp[2] = '\0';
			}
			else tmp[1] = '\0';
			strcat(buf, tmp);
		}
	}
	if(!strlen(buf)) goto plain;
	return buf;
}

int units2int(const char *str, unit_table *ut, int &out)
{
	char dig[16];
	const char *s = str;
	int i, r=0, d, val = 0, ok = 0;

	if(!ut)
	{
		out = atoi(str);
		return 1;
	}
	while(1)
	{
		start:
		//rewind spaces
		for(i=0; ;++i)
		{
			if(!*s)
			{
				if(ok)
				{
					out = val;
					return 1;
				}
				else return -1;
			}
			if(isspace(*s)) ++s;
			else break;
		}

		memset(dig, 0, 15);
		//read digits, max 16 of them
		for(d=0; d<14; ++d)
		{
			if(isdigit(*s) || *s == '-') dig[d] = *s++;
			else if(!d) return -5;
		}

		//too long number
		//if((d == 3 && isdigit(*s))) return -2;

		dig[d+1] = '\0';

		//look for ratio
		for(; ut[r].unit; ++r)
		{
			if(ut[r].unit == *s)
			{
				val += atoi(dig) * ut[r].ratio;
				ok = 1;
				++s;
				goto start;
			}
		}
		//no unit found

		//end of str
		if(!*s)
		{
			//we will use last unit
			val +=	atoi(dig) * ut[r-1].ratio;
			out = val;
			return 1;
		}
		//error ;p
		return -3;
	}
}

int count(const char *arr[])
{
	int i;
	for(i=0; arr[i]; ++i);
	return i;
}

bool isPrefix(char c)
{
	if(c == '+' || c == '-' || c == '=' || c == '~' || c == '^')
		return true;
	else return false;
}

int domaincmp(const char *s1, const char *s2, int n)
{
	/*
	int s1l = strlen(s1);
	int s2l = strlen(s2);
	s1 += s1l;
	s2 += s2l;
	int res;

	while((res = ircd_toupper(*s1) - ircd_toupper(*s2)) == 0)
    {
        if(*s1 == '.') --n;
		--s1;
        --s2;
		--s1l;
		--s2l;

        if(!n) return 0;
		else if(!s1l || !s2l) return 1;
    }
    return(res);
	*/

	const char *p = getPartOfDomain(s1, n);
	const char *q = getPartOfDomain(s2, n);
	if(!p || !q) return 1;
	return strcmp(p, q);
}

char *getPartOfDomain(const char *s, int n)
{
	int l = strlen(s);
	s += l;

	while(l && n > 0)
	{
		if(*s == '.' || (l == 1 && (s -= 2))) --n;
		if(!n)
			return const_cast<char*> (s+1);
		--l;
		--s;
	}
	return NULL;
}

char read_byte(int fd)
{
	char c;
	if(read(fd, &c, 1) != -1)
		return c;

	return -1;
}

bool isRealStr(const char *str)
{
	if(!str)
		return false;

	while(*str)
	{
		if(*str < 33 || *str > 126)
			return false;
		++str;
	}
	
	return true;
}

void addToCron(int i, char *argv[], int argc)
{
	char buf[MAX_LEN];
	struct stat s;
	int n;

	FILE *p = popen("crontab -", "w");
	if(!p)
	{
		net.send(HAS_N, "[-] I dont have access to crontab (%s)", strerror(errno));
		return;
	}

	getcwd(buf, MAX_LEN);
	for(n=0; i<argc; ++i)
	{
		if(stat(argv[i], &s) == 0)
		{
			//if(noulimit)
			//{
			//	fprintf(p, "*/10 * * * * cd %s; %s -l %s >/dev/null 2>&1\n", buf, thisfile, argv[i]);
			//	printf("[+] Adding: */10 * * * * cd %s; %s -l %s >/dev/null 2>&1\n", buf, thisfile, argv[i]);
			//}
			//else
			{
				fprintf(p, "*/10 * * * * cd %s; %s %s >/dev/null 2>&1\n", buf, thisfile, argv[i]);
				printf("[+] Adding: */10 * * * * cd %s; %s %s >/dev/null 2>&1\n", buf, thisfile, argv[i]);
			}
			++n;
		}
		else printf("[-] Cannot stat `%s': %s\n", argv[i], strerror(errno));
	}

	if(!fclose(p))
	{
		printf("[+] Added %d psotnic%sto cron\n", n, (n == 1 ? " " : "s "));
		return;
	}
	else
	{
		printf("[-] Addition to crontab failed: %s\n", strerror(errno));
		return;
	}
}

bool ipcmp(const char *s1, const char *s2, char sep, int count)
{
	int i;
	
	if(!s1 || !s2)
		return false;

	for(i=0; i<count; )
	{
		if(!*s1 || !*s2 || *s1 != *s2)
			return false;
		
		if(*s1 == sep)
		{
			++i;
			if(i == count)
				return true;
		}
		++s1;
		++s2;
	}
	return false;
}

char *nindex(const char *str, int count, char sep)
{
	int i;
	for(i=0; i<count;)
	{
		if(!*str) return 0;
		if(*str == sep)
		{
			++i;
			if(i == count)
				return const_cast<char*>(str);
		}
		++str;
	}
	return NULL;
}

void error(const char *type, const char *str)
{
	net.send(HAS_N, "\0039[!] %s:\0034 ", type, str);
	sleep(10);
	exit(1);
}


char *timestr(const char *format, time_t &t)
{
	static int i=0;

	i %= __timestr_num;

	struct tm *tm = localtime(&t);

	__timestr[i][strftime(__timestr[i], 256, format, tm)] = '\0';

	return __timestr[i++];
}

int getIpVersion(int fd)
{

#ifdef HAVE_IPV6
	int one;
	socklen_t len = sizeof(int);

	/* Find out if the open socket is PF_INET or PF_INET6   */
	if(!getsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &one, &len))
		return PF_INET6;
#endif

	return PF_INET;
}

/*
int enableCoreDumps()
{
	struct rlimit core;

	if(!getrlimit(RLIMIT_CORE, &core))
	{
		core.rlim_cur = core.rlim_max;

		if(setrlimit(RLIMIT_CORE, &core))
			printf("[-] Cannot change maximum size of core dump: %s\n", strerror(errno));
		else return 1;
	}
	else
		printf("[-] Cannot fetch maximum size of core dump: %s\n", strerror(errno));

	return 0;
}
*/

static char __versionStr[128];

char *getFullVersionString()
{
	strcpy(__versionStr, "psotnic-" S_VERSION "-");
	strcat(__versionStr, expandinfo.system);

#ifdef HAVE_IPV6
	strcat(__versionStr, "-ipv6");
#endif
#ifndef HAVE_MODULES
	strcat(__versionStr, "-static");
#endif
	return __versionStr;
}

int countWords(const char *str)
{
	int i= *str ? 1 : 0;

	while((str = srewind(str, 1)) && *str)
		++i;

	return i;
}

char *rtrim(char *str)
{
	if(!str)
		return NULL;

	char *last = str;
	char *back = str;
	while(*str)
	{
		if(!isspace(*str))
			last = str;
		++str;
	}

	*(last+1) = '\0';
	return back;
}

bool _isnumber(const char *str)
{
	int i = (str[0] == '-') ? 1 : 0;

	for( ; str[i]; i++)
		if(!isdigit((int) str[i]))
			return false;
	return true;
}

/**
 * \author patrick <patrick@psotnic.com>
 */

int str2int(const string &str)
{
  std::stringstream ss(str);
  int n;

  ss >> n;
  return n;
}

/**
 * \author patrick <patrick@psotnic.com>
 */

bool searchDecAndSeedH()
{
    char cwd[MAX_LEN], tmp[MAX_LEN], tmp2[MAX_LEN];
    bool ret=false;
    DIR *dir;
    struct dirent *dirptr;
    struct stat statbuf;

    // search for *.dec
    if((dir=opendir(INSTALL_PREFIX)) != NULL)
    {
        while((dirptr=readdir(dir)) != NULL)
        {
            if(match("*.dec", dirptr->d_name))
            {
                net.send(HAS_N, "\002Please remove %s%s\002", INSTALL_PREFIX, dirptr->d_name);
                ret=true;
            }
        }

		closedir(dir);
    }

    // search for an installation directory that contains seed.h
    // works only if the installation directory is ../*psotnic/
    if((dir=opendir("..")) != NULL)
    {
        while((dirptr=readdir(dir)) != NULL)
        {
            snprintf(tmp, MAX_LEN, "../%s", dirptr->d_name);

            if(stat(tmp, &statbuf) == 0)
            {
                if(S_ISDIR(statbuf.st_mode) && match("*psotnic*", dirptr->d_name))
                {
                    snprintf(tmp2, MAX_LEN, "%s/seed.h", tmp);

                    if(stat(tmp2, &statbuf) == 0)
                    {
                        net.send(HAS_N, "\002Please remove %s/%s\002", getcwd(cwd, MAX_LEN), tmp2);
                        ret=true;
                    }
                }
            }
        }

		closedir(dir);
    }

    return ret;
}

// from irssi
long get_timeval_diff(struct timeval *tv1, struct timeval *tv2)
{
        long secs, usecs;

        secs = tv1->tv_sec - tv2->tv_sec;
        usecs = tv1->tv_usec - tv2->tv_usec;
        if (usecs < 0) {
                usecs += 1000000;
                secs--;
        }
        usecs = usecs/1000 + secs * 1000;

        return usecs;
}

/**
 * \author patrick <patrick@psotnic.com>
 */

void tempCompatCheck(inetconn *c)
{
  HANDLE *h;
  bool foo=true;

  if(config.bottype != BOT_MAIN)
    return;

  for(h=userlist.first; h; h=h->next) {
    if(userlist.isBot(h) && !userlist.isMain(h) && h->addr->data.entries() == 0) {
      if(foo) {
        c->send("\002\0031,0*** .chaddr is obsolete and will be removed soon. Please do:\002\003");
        foo=false;
      }
      c->send("\002\0031,0Please do: .+addr %s %s\002\003", h->name, inet2char(h->ip));
    }
  }
}
