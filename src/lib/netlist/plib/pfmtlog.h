// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pfmtlog.h
 */

#ifndef PFMT_H_
#define PFMT_H_

#include "pstring.h"
#include "ptypes.h"

#include <limits>

namespace plib {

P_ENUM(plog_level,
	DEBUG,
	INFO,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL)

template <typename T>
struct ptype_traits_base
{
	static const T cast(const T &x) { return x; }
	static const bool is_signed = std::numeric_limits<T>::is_signed;
	static const char *size_spec() { return ""; }
	static char32_t fmt_spec() { return 'u'; }
};

template <>
struct ptype_traits_base<bool>
{
	static unsigned int cast(bool &x) { return static_cast<unsigned int>(x); }
	static unsigned int cast(const bool &x) { return static_cast<unsigned int>(x); }
	static const bool is_signed = std::numeric_limits<bool>::is_signed;
	static const char *size_spec() { return ""; }
	static char32_t fmt_spec() { return 'u'; }
};

template <typename T>
struct ptype_traits;

template<>
struct ptype_traits<bool> : ptype_traits_base<bool>
{
};

template<>
struct ptype_traits<char> : ptype_traits_base<char>
{
	static const char *size_spec() { return "h"; }
	static char32_t fmt_spec() { return is_signed ? 'd' : 'u'; }
};

template<>
struct ptype_traits<short> : ptype_traits_base<short>
{
	static const char *size_spec() { return "h"; }
	static char32_t fmt_spec() { return 'd'; }
};

template<>
struct ptype_traits<int> : ptype_traits_base<int>
{
	static char32_t fmt_spec() { return 'd'; }
};

template<>
struct ptype_traits<long> : ptype_traits_base<long>
{
	static const char *size_spec() { return "l"; }
	static char32_t fmt_spec() { return 'd'; }
};

template<>
struct ptype_traits<long long> : ptype_traits_base<long long>
{
	static const char *size_spec() { return "ll"; }
	static char32_t fmt_spec() { return 'd'; }
};

template<>
struct ptype_traits<signed char> : ptype_traits_base<signed char>
{
	static const char *size_spec() { return "h"; }
	static char32_t fmt_spec() { return 'd'; }
};

template<>
struct ptype_traits<unsigned char> : ptype_traits_base<unsigned char>
{
	static const char *size_spec() { return "h"; }
	static char32_t fmt_spec() { return 'u'; }
};

template<>
struct ptype_traits<unsigned short> : ptype_traits_base<unsigned short>
{
	static const char *size_spec() { return "h"; }
	static char32_t fmt_spec() { return 'u'; }
};

template<>
struct ptype_traits<unsigned int> : ptype_traits_base<unsigned int>
{
	static char32_t fmt_spec() { return 'u'; }
};

template<>
struct ptype_traits<unsigned long> : ptype_traits_base<unsigned long>
{
	static const char *size_spec() { return "l"; }
	static char32_t fmt_spec() { return 'u'; }
};

template<>
struct ptype_traits<unsigned long long> : ptype_traits_base<unsigned long long>
{
	static const char *size_spec() { return "ll"; }
	static char32_t fmt_spec() { return 'u'; }
};

template<>
struct ptype_traits<float> : ptype_traits_base<float>
{
	static char32_t fmt_spec() { return 'f'; }
};

template<>
struct ptype_traits<double> : ptype_traits_base<double>
{
	static char32_t fmt_spec() { return 'f'; }
};


template<>
struct ptype_traits<char *> : ptype_traits_base<char *>
{
	static const char *cast(const char *x) { return x; }
	static char32_t fmt_spec() { return 's'; }
};

class pfmt
{
public:
	explicit pfmt(const pstring &fmt)
	: m_str(fmt), m_arg(0)
	{
	}

	~pfmt()
	{
	}

	operator pstring() const { return m_str; }
	pfmt &          e(const double &x) {return format_element("", 'e', x);  }
	pfmt &          g(const double &x) {return format_element("", 'g', x);  }

	pfmt &          e(const float &x) {return format_element("", 'e', static_cast<double>(x));  }
	pfmt &          g(const float &x) {return format_element("", 'g', static_cast<double>(x));  }

	pfmt &operator ()(const void *x) {return format_element("", 'p', x);  }
	pfmt &operator ()(const pstring &x) {return format_element("", 's', x.c_str() );  }

	template<typename T>
	pfmt &operator ()(const T &x)
	{
		return format_element(ptype_traits<T>::size_spec(), ptype_traits<T>::fmt_spec(), ptype_traits<T>::cast(x));
	}

	template<typename T>
	pfmt &operator ()(const T *x)
	{
		return format_element(ptype_traits<T *>::size_spec(), ptype_traits<T *>::fmt_spec(), ptype_traits<T *>::cast(x));
	}

	template<typename T>
	pfmt &x(const T &x)
	{
		return format_element(ptype_traits<T>::size_spec(), 'x', x);
	}

	template<typename T>
	pfmt &o(const T &x)
	{
		return format_element(ptype_traits<T>::size_spec(), 'o', x);
	}
protected:

	pfmt &format_element(const char *l, const unsigned fmt_spec, ...);

private:

	pstring m_str;
	unsigned m_arg;
};

class plog_dispatch_intf;

template <bool build_enabled = true>
class pfmt_writer_t : plib::nocopyassignmove
{
public:
	explicit pfmt_writer_t() : m_enabled(true)  { }

	/* runtime enable */
	template<bool enabled, typename... Args>
	void log(const pstring & fmt, Args&&... args) const
	{
		if (build_enabled && enabled && m_enabled)
			vdowrite(xlog(pfmt(fmt), std::forward<Args>(args)...));
	}

	template<typename... Args>
	void operator ()(const pstring &fmt, Args&&... args) const
	{
		if (build_enabled && m_enabled)
		{
			pfmt pf(fmt);
			vdowrite(xlog(pf, std::forward<Args>(args)...));
		}
	}

	void set_enabled(const bool v)
	{
		m_enabled = v;
	}

	bool is_enabled() const { return m_enabled; }

protected:
	~pfmt_writer_t() { }
	virtual void vdowrite(const pstring &ls) const = 0;

private:
	pfmt &xlog(pfmt &fmt) const { return fmt; }

	template<typename X, typename... Args>
	pfmt &xlog(pfmt &fmt, X&& x, Args&&... args) const
	{
		return xlog(fmt(std::forward<X>(x)), std::forward<Args>(args)...);
	}

	bool m_enabled;

};

template <plog_level::E L, bool build_enabled = true>
class plog_channel : public pfmt_writer_t<build_enabled>
{
public:
	explicit plog_channel(plog_dispatch_intf *b) : pfmt_writer_t<build_enabled>(), m_base(b) { }
	virtual ~plog_channel() { }

protected:
	virtual void vdowrite(const pstring &ls) const override;

private:
	plog_dispatch_intf *m_base;
};

class plog_dispatch_intf
{
	template<plog_level::E, bool> friend class plog_channel;

public:
	virtual ~plog_dispatch_intf();
protected:
	virtual void vlog(const plog_level &l, const pstring &ls) const = 0;
};

template<bool debug_enabled>
class plog_base
{
public:

	explicit plog_base(plog_dispatch_intf *proxy)
	: debug(proxy),
		info(proxy),
		verbose(proxy),
		warning(proxy),
		error(proxy),
		fatal(proxy)
	{}
	virtual ~plog_base() {}

	plog_channel<plog_level::DEBUG, debug_enabled> debug;
	plog_channel<plog_level::INFO> info;
	plog_channel<plog_level::VERBOSE> verbose;
	plog_channel<plog_level::WARNING> warning;
	plog_channel<plog_level::ERROR> error;
	plog_channel<plog_level::FATAL> fatal;
};


template <plog_level::E L, bool build_enabled>
void plog_channel<L, build_enabled>::vdowrite(const pstring &ls) const
{
	m_base->vlog(L, ls);
}

}


#endif /* PSTRING_H_ */
