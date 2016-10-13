#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <streambuf>
#include <algorithm>
#include <functional>
#include <regex>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <string.h>
#include <glob.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <magic.h>
#include <string.h>
#include <iconv.h>
#include <random>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream> // TODO Debug remove

namespace util
{
	// String manipulation

	const std::string ucslocale = "en_US.UTF-8";

	std::string strjoin(const std::vector<std::string> &list, char delim, unsigned int start = 0, unsigned int end = 0);

	std::vector<std::string> strsplit(const std::string &str, char delim);

	std::vector<std::string> argvec(int argc, char **argv);

	std::pair<std::unordered_map<std::string, std::vector<std::string>>, std::vector<std::string>> argmap(unsigned int argc, char **argv, const std::string &valid, bool stop = false);

	std::string alnumonly(const std::string &str);

	std::string utf8lower(const std::string &str);

	int fast_atoi(const char *s);

	int fast_atoi(const std::string &s);

	std::string conv(const std::string &in, const std::string &from, const std::string &to);

	template <typename It, typename T> It find_nth(It begin, It end, const T& query, unsigned int n)
	{
		if (n == 0) return begin;
		unsigned int cnt = 0;
		for (It i = begin; i != end; i++) if (*i == query)
		{
			if (++cnt == n) return i;
		}
		return end;
	}

	template <typename T> std::string t2s(const T &t)
	{
		std::stringstream ret{};
		ret << t;
		return ret.str();
	}

	template <typename T> T s2t(const std::string &s)
	{
		std::stringstream ss{};
		ss << s;
		T ret;
		ss >> ret;
		return ret;
	}


	// Math

	template <typename T> T randint(T min, T max)
	{
		static std::default_random_engine dre{std::random_device{}()}; // FIXME Probably not thread-safe
		std::uniform_int_distribution<T> dist{min, max};
		return dist(dre);
	}


	// Container operations

	template <typename T> std::unordered_set<T> intersection(const std::unordered_set<T> &s1, const std::unordered_set<T> &s2)
	{
		std::unordered_set<T> ret{};
		for (const T &t : s1) if (s2.count(t)) ret.insert(t);
		return ret;
	}

	template <typename T> std::unordered_set<T> difference(const std::unordered_set<T> &s1, const std::unordered_set<T> &s2)
	{
		std::unordered_set<T> ret{};
		for (const T &t : s1) if (! s2.count(t)) ret.insert(t);
		return ret;
	}

	template <typename T> std::unordered_set<T> set_union(const std::unordered_set<T> &s1, const std::unordered_set<T> &s2)
	{
		std::unordered_set<T> ret{};
		for (const T &t : s1) ret.insert(t);
		for (const T &t : s2) ret.insert(t);
		return ret;
	}


	// Filesystem

	const char pathsep = '/';

	std::string pathjoin(const std::vector<std::string> &list);

	std::string basename(std::string path, char sep = '/');

	std::string dirname(std::string path);

	void rm(const std::string &path);

	void rm_recursive(const std::string &path);

	std::string exepath();

	std::size_t fsize(const std::string &path);

	bool fexists(const std::string &path);

	bool isdir(const std::string &path);

	void fswalk(const std::string &path, const std::function<bool(const std::string &, const struct stat *, void *)> &fn, void *userd, bool follow = false);

	std::unordered_set<std::string> ls(const std::string &dir, const std::string &test = "");

	std::unordered_set<std::string> recursive_ls(const std::string &base, const std::string &test = "");

	std::string normalize(const std::string &path);

	std::string resolve(const std::string &base, const std::string &path);

	std::string linktarget(std::string path);

	std::string relreduce(std::string base, std::string target);

	bool is_under(std::string parent, std::string child);


	// Internet

	std::string ext2mime(const std::string &path);

	std::string mimetype(const std::string &path, const std::string &data);

	std::string mimetype(const std::string &path);

	std::string urlencode(const std::string &str);

	std::string urldecode(const std::string &str);

	std::string from_htmlent(const std::string &str);

	std::string to_htmlent(const std::string &str);


	// Time

	class timer
	{
	private:
		std::chrono::time_point<std::chrono::system_clock> last;
	public:
		void reset() { last = std::chrono::system_clock::now(); }
		double check(bool reset = false)
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed = now - last;
			if (reset) this->reset();
			return elapsed.count();
		};
		timer() : last{} { reset(); }
	};

	std::string timestr(const std::string &fmt = "%c", std::time_t time = std::time(nullptr));


	// Streams

	class rangebuf : public std::streambuf
	{
	private:
		static const size_t blocksize = 32 * 1024;
		std::istream &src_;
		std::vector<char> buf_;
		std::streampos start_, size_, pos_;
		std::streamsize fill();
	public:
		rangebuf() = delete; //: file_{}, start_{0}, size{0};
		rangebuf(std::istream &src, std::streampos start, std::streampos size);
		rangebuf(const rangebuf &orig) = delete;
		int_type underflow();
		pos_type seekpos(pos_type target, std::ios_base::openmode which);
		pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which);
	};

	std::streampos streamsize(std::istream &stream);


	// Memory

	class mmap_guard
	{
	private:
		int fd;
		size_t fsize;
		void *map;
	public:
		void *open(const std::string &fname);
		void *get() { return map; }
		void close();
		size_t size() { return fsize; }
		mmap_guard() : fd{0}, fsize{0}, map{nullptr} { }
		mmap_guard(const std::string &fname) : map{nullptr} { open(fname); }
		mmap_guard(const mmap_guard &orig) = delete;
		mmap_guard(mmap_guard &&orig) : fd{orig.fd}, fsize{orig.fsize}, map{orig.map} { orig.map = nullptr; }
		virtual ~mmap_guard() { close(); }
	};
}

#endif

