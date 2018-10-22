#include "html.h"

namespace html
{
	std::string xmlc2str(const xmlChar *s) { return std::string{reinterpret_cast<const char *>(s)}; } // TODO Why is reinterpret necessary?
	const xmlChar *str2xmlc(const std::string &s) { return reinterpret_cast<const xmlChar *>(s.c_str()); }

	std::string tag::name()
	{
		return xmlc2str(node_->name);
	}

	std::unordered_map<std::string, std::string> tag::props()
	{
		std::unordered_map<std::string, std::string> ret{};
		for (xmlAttr *attr = node_->properties; attr; attr = attr->next)
		{
			ret[xmlc2str(attr->name)] = xmlc2str(attr->children->content);
		}
		return ret;
	}

	std::string tag::content()
	{
		return xmlc2str(node_->content);
	}

	std::vector<tag> tag::children()
	{
		std::vector<tag> ret{};
		for (xmlNode *child = node_->children; child; child = child->next)
		{
			ret.push_back(tag{child});
		}
		return ret;
	}

	doc doc::parse_string(const std::string &in)
	{
		return doc{htmlReadDoc(str2xmlc(in), NULL, NULL, parse_opts)};
	}

	doc doc::read_file(const std::string &fname)
	{
		return doc{htmlReadFile(fname.c_str(), NULL, parse_opts)};
	}

	std::vector<tag> doc::xpath(const std::string &path)
	{
		xmlXPathContext *ctx = xmlXPathNewContext(doc_);
		if (! ctx) throw std::runtime_error{"Failed to create XPath context"};
		xmlXPathObject *obj = xmlXPathEvalExpression(str2xmlc(path), ctx);
		if (! obj)
		{
			xmlXPathFreeContext(ctx);
			throw std::runtime_error{"Failed to parse XPath expression " + path};
		}
		if (obj->type != XPATH_NODESET)
		{
			xmlXPathFreeContext(ctx);
			throw std::runtime_error{"XPath object type " + util::t2s(obj->type) + " is not supported"}; // TODO
		}
		std::vector<tag> ret{};
		if (! obj->nodesetval) return ret;
		for (int i = 0; i < obj->nodesetval->nodeNr; i++) ret.push_back(tag{obj->nodesetval->nodeTab[i]});
		xmlXPathFreeContext(ctx);
		xmlXPathFreeObject(obj);
		return ret;
	}

	std::string doc::title()
	{
		std::vector<tag> matches = xpath("/html/head/title/text()");
		if (matches.empty()) return "";
		return matches[0].content();
	}

	std::string doc::encoding()
	{
		const xmlChar *lib = htmlGetMetaEncoding(doc_);
		if (lib) return xmlc2str(lib);
		std::vector<tag> meta = xpath("//meta/@charset");
		if (meta.size() > 0) return meta[0].children()[0].content();
		std::vector<tag> http = xpath("//meta[@http-equiv='Content-Type']/@content");
		if (http.size() > 0)
		{
			std::string value = http[0].children()[0].content();
			std::regex charset{"charset=([a-zA-Z0-9_-]+)"};
			std::smatch res{};
			if (std::regex_search(value, res, charset)) return res[1];
		}
		return "latin-1";
	}

	doc::~doc()
	{
		xmlFreeDoc(doc_);
	}
}
