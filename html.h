#ifndef UTIL_HTML_H
#define UTIL_HTML_H
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <stdexcept>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include "util.h"

// TODO
// https://stackoverflow.com/questions/11901206

namespace html
{
	class tag
	{
	private:
		xmlNode *node_;
	public:
		tag(xmlNode *n) : node_{n} { }
		std::string name();
		std::unordered_map<std::string, std::string> props();
		std::string content();
		std::vector<tag> children();
	};

	class doc
	{
	private:
		static constexpr int parse_opts = HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
		xmlDoc *doc_;
		doc(xmlDoc *d): doc_{d} { }
	public:
		static doc parse_string(const std::string &in);
		static doc read_file(const std::string &fname);
		std::vector<tag> xpath(const std::string &path);
		std::string title();
		std::string encoding();
		virtual ~doc();
	};
}

#endif
