#ifndef PKS_CLASS_ENT_H
#define PKS_CLASS_ENT_H 1

#include "pstring.h"
#include "ptrlist.h"
#include "prots.h"
#include "common.h"

class ent;
class inetconn;

class options
{
	public:
	class event
	{
		public:
		pstring<> reason;
		bool ok;
		bool notFound;
		ent *entity;

		void setOk(ent *e, const char *format, ...);
		void setError(ent *e, const char *format, ...);
		void setError(ent *e);
		void setNotFound(const char *format, ...);
		event();
	};

	ptrlist<ent> list;

	options();

	event *setVariable(const char *var, const char *value);
	const char *getValue(const char *var);
	void sendToOwner(const char *owner, const char *var, const char *prefix);
	void sendToSTDout(const char *var, const char *prefix);
	bool parseUser(const char *from, const char *var, const char *value, const char *prefix, const char *prefix2="");
	bool parseUserSTDout(const char *var, const char *value, const char *prefix, const char *prefix2="");
	void reset();
	void sendToFile(inetconn *c, pstring<> prefix);

#ifdef HAVE_DEBUG
	void display();
#endif

	protected:
	void registerObject(const ent &e);
	int maxVarLen;
};

class ent
{
	public:
	const char *name;
	static options::event _event;
	bool dontPrintIfDefault;
	bool readOnly;

	ent(const char *n=NULL) : name(n), dontPrintIfDefault(false), readOnly(false) { };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0) = 0;
	virtual options::event *set(const char *arg, const bool justTest=0);
	virtual const char *getValue() const = 0 ;
	virtual const char *getName() const;
	virtual const char *print(const int n=0) const;
	virtual void reset() = 0;
	virtual bool isDefault()  const = 0;
	virtual bool isPrintable() const;
	virtual bool isReadOnly() const { return readOnly; };
	virtual void setDontPrintIfDefault(bool value) { dontPrintIfDefault = value; };
	virtual void setReadOnly(bool value) { readOnly = value; };
	bool operator<(const ent &e) const;
	bool operator==(const ent &e) const;

	//virtual ~ent();
};


class entBool : public ent
{
	public:
	int value;
	int defaultValue;

	virtual options::event *setValue(const char *arg1, const char *arg2="", const bool justTest=0);
	operator int() const;
	entBool() : ent(NULL), value(false), defaultValue(false) { };
	entBool(const char *n, const bool def) : ent(n), value(def), defaultValue(def) { };

	bool str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	int operator=(int n)	{ value = (n == 1); return value; };
	virtual ~entBool() { };
};

class entInt : public entBool
{
	public:
	int min, max;

	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual int str2int(const char *str, bool &ok) const;
	entInt() : min(0), max(0) { name = NULL; };
	entInt(const char *n, const int minimum, const int maximum, const int def)
		: min(minimum), max(maximum) { name = n, value = defaultValue = def; };
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	operator int() const;
	int operator==(int n) const;
	virtual ~entInt() { };
};

class entTime : public entInt
{
	public:
	entTime() : entInt() { };
	entTime(const char *n, const int minimum, const int maximum, const int def) :
		entInt(n, minimum, maximum, def) { };

	virtual int str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	virtual ~entTime() { };
};

class entPerc : public entInt
{
	public:
	entPerc() : entInt() { };
	entPerc(const char *n, const int minimum, const int maximum, const int def) :
		entInt(n, minimum, maximum, def) { };

	virtual int str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	virtual ~entPerc() { };
};

class entHost : public ent
{
	protected:
	int typesAllowed;

	public:
	pstring<8> ip;
	pstring<8> ip4;
	pstring<64> ip6; // FIXME
	pstring<16> connectionString;
#ifdef HAVE_ADNS
	time_t resolve_pending; /// time when the last resolve attempt was started
				/// will be set to 0 when resolution is finish (success or error)
#endif
	enum types { ipv4 = 0x01, ipv6 = 0x02, bindCheck = 0x04, domain = 0x08, use_ssl = 0x40, use_ipv4 = 0x20, use_ipv6 = 0x10 };
	entHost(const char *n="", const int t=ipv4) : ent(n), typesAllowed(t), ip("0.0.0.0"), connectionString("0.0.0.0") { };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual operator const char*() const;
	virtual operator unsigned int() const;
	virtual ~entHost() { };
	bool isSSL() { return getConnectionStringType(connectionString) & use_ssl; };
	bool isIpv4() { return getConnectionStringType(connectionString) & ipv4; };
	bool isIpv6() { return getConnectionStringType(connectionString) & ipv6; };
	int getConnectionStringType(const char *str=0) const;
#ifdef HAVE_ADNS
	int updateDnsEntry();
#endif
};

class entString : public ent
{
	public:
	int min, max;
	pstring <16> string;
	pstring <16> defaultString;


	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual operator const char*() const;
	virtual int getLen() const;
	entString(const char *n="", const int minimum=0, const int maximum=MAX_INT, const char *def="") :
			ent(n), min(minimum), max(maximum), string(def), defaultString(def) { }
	virtual ~entString() { };
};

class entWord : public entString
{
	public:
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
		entWord(const char *n="", const int minimum=0, const int maximum=MAX_INT, const char *def="")
	: entString(n, minimum, maximum, def) { };

	virtual ~entWord() { };
};

class entMD5Hash : public entWord
{
	public:
	unsigned char hash[16];

	public:
	entMD5Hash(const char *n="") : entWord(n)	{ memset(hash, 0, 16); };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual void reset();
	virtual bool isDefault() const;
	virtual const unsigned char *getHash() const;
	virtual operator const unsigned char*() const;
	virtual ~entMD5Hash() { };
};

class entHPPH : public ent	//host port password handle
{
	protected:
	entHost *_host;
	entInt *_port;
	entWord *_pass;
	entWord *_handle;
	virtual options::event *_setValue(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const bool justTest);

	public:
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	entHPPH(const char *n="", entHost *host=0, entInt *port=0, entWord *pass=0, entWord *handle=0) :
			ent(n), _host(host), _port(port), _pass(pass), _handle(handle) { };
	virtual const char *getValue() const;
	virtual ~entHPPH();
	virtual entHost &getHost()		{ return *_host; };
	virtual entInt &getPort()	{ return *_port; };
	virtual entWord &getPass()	{ return *_pass; };
	virtual entWord &getHandle(){ return *_handle; };
	virtual entHPPH &operator=(const entHPPH &e);
	virtual void reset();
	virtual bool isDefault() const;

	friend class CONFIG;
	virtual bool isSSL() const { return _host->isSSL(); };
};

class entHub : public entHPPH
{
	private:

	public:
	int failures;
	entHub(const char *n="", entHost *host=0, entInt *port=0, entMD5Hash *pass=0, entWord *handle=0) :
		entHPPH(n, host, port, pass, handle), failures(0) { };
	virtual ~entHub() { };
};


class entServer : public entHPPH
{
	public:
	entServer(const char *n="", entHost *ip=0, entInt *port=0, entWord *pass=0) :
		entHPPH(n, ip, port, pass, 0) { };
	virtual options::event *set(const char *ip, const char *port, const char *pass="", const bool justTest=0);
	virtual ~entServer() { };
};

class entListener : public entHPPH
{
	public:
	entListener(const char *n="", entHost *ip=0, entInt *port=0, entWord *opt=0) :
		entHPPH(n, ip, port, 0, opt) { };
	virtual options::event *set(const char *arg, const bool justTest=0);
	virtual ~entListener() { };
};

class entMult : public ent
{
	private:
	ptrlist<ent> list;

	public:
	entMult(const char *n=""): ent(n) { };
	virtual options::event *setValue(const char *arg1, const char *arg2, bool justTest=0);
	virtual const char *getValue() const { return NULL; };
	virtual void reset();
	virtual bool isDefault() const { return 1; };
	virtual bool isPrintable() const { return 0; };
	virtual void add(ent *e);
	virtual ~entMult() { };
};

class entLoadModules : public ent
{
	private:
	pstring<> file;
	pstring<> md5sum;
	bool md5;

	virtual options::event *_setValue(const char *arg1, const char *arg2, const char *arg3, bool justTest=0);

	public:
		entLoadModules(const char *n="", bool needValidMD5=1) : ent(n), md5(needValidMD5) { };
	virtual ~entLoadModules() { };

	virtual options::event *setValue(const char *arg1, const char *arg2, bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual entLoadModules &operator=(const entLoadModules &e);
	ptrlist<module>::iterator findModule(const char *str);
	//bool rehash
	bool unload(const char *str);
};

#endif
