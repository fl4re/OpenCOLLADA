#include "XmlNode.h"
#include "XmlAttribute.h"
#include "XmlDoc.h"
#include "XmlNamespace.h"
#include "XmlNodeSet.h"
#include "XmlNodes.h"
#include "libxml/xpathInternals.h"

#include "no_warning_string"

using namespace std;

namespace opencollada
{
	XmlNode::XmlNode(xmlNodePtr node)
		: mNode(node)
	{}

	XmlNode::operator bool() const
	{
		return mNode != nullptr;
	}

	XmlDoc & XmlNode::doc() const
	{
		return XmlDoc::GetXmlDoc(mNode->doc);
	}

	XmlNode XmlNode::child(const string & name) const
	{
		for (xmlNodePtr node = mNode->children; node; node = node->next)
		{
			if (node->type == XML_ELEMENT_NODE &&
				name == node->name)
			{
				return XmlNode(node);
			}
		}
		return XmlNode();
	}

	XmlNode XmlNode::firstChild() const
	{
		return XmlNode(mNode->children->next);
	}

	string XmlNode::text() const
	{
		for (xmlNodePtr node = mNode->children; node; node = node->next)
		{
			if (node->type == XML_TEXT_NODE)
			{
				return reinterpret_cast<const XmlChar*>(node->content);
			}
		}
		return string();
	}

	string XmlNode::name() const
	{
		return reinterpret_cast<const XmlChar*>(mNode->name);
	}

	XmlAttribute XmlNode::attribute(const string & name) const
	{
		for (xmlAttrPtr attr = mNode->properties; attr; attr = attr->next)
		{
			if (name == attr->name)
			{
				return XmlAttribute(attr);
			}
		}
		return XmlAttribute();
	}

	XmlNamespace XmlNode::ns() const
	{
		return XmlNamespace(mNode->ns);
	}

	class ScopedSetDocRoot
	{
	public:
		ScopedSetDocRoot(const XmlNode & node)
			: mDoc(node.doc())
			, mRoot(mDoc.root())
		{
			mDoc.setRoot(node);
		}

		~ScopedSetDocRoot()
		{
			mDoc.setRoot(mRoot);
		}

	private:
		const ScopedSetDocRoot & operator = (const ScopedSetDocRoot &) = delete;

	private:
		XmlDoc & mDoc;
		XmlNode mRoot;
	};

	const XmlNodeSet & XmlNode::selectNodes(const string & xpath) const
	{
		auto & xpathCache = XmlDoc::GetXmlDoc(mNode->doc).mXPathCache;
		auto cache = xpathCache.find(XPathCacheKey(mNode, xpath));
		if (cache != xpathCache.end())
			return cache->second;

		ScopedSetDocRoot ssdr(*this);

		if (xmlXPathContextPtr context = xmlXPathNewContext(mNode->doc))
		{
			xmlXPathRegisterNs(context, BAD_CAST "collada", BAD_CAST "http://www.collada.org/2005/11/COLLADASchema");
			xmlXPathRegisterNs(context, BAD_CAST "lod", BAD_CAST "http://www.collada.org/2016/08/COLLADALODSchema");
			xmlXPathRegisterNs(context, BAD_CAST "xsi", BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");

			XmlNodeSet result(xmlXPathEvalExpression(BAD_CAST xpath.c_str(), context));
			xmlXPathFreeContext(context);
			auto p = xpathCache.insert(pair<XPathCacheKey, XmlNodeSet>(XPathCacheKey(mNode, xpath), move(result)));
			return p.first->second;
		}
		return XmlNodeSet::null;
	}

	XmlNodes<XmlNodeIteratorByName> XmlNode::children(const string & name) const
	{
		return XmlNodes<XmlNodeIteratorByName>(XmlNodeIteratorByName(child(name), name), XmlNodeIteratorByName());
	}

	XmlNode XmlNode::next(const string & name) const
	{
		xmlNodePtr next = mNode->next;
		while (next && name == mNode->next->name)
		{
			next = mNode->next;
		}
		return XmlNode(next);
	}

	size_t XmlNode::line() const
	{
		return mNode->line;
	}

	void XmlNode::for_each_attribute(const function<void(const XmlAttribute &)> & fn) const
	{
		for (xmlAttrPtr attr = mNode->properties; attr; attr = attr->next)
		{
			fn(XmlAttribute(attr));
		}
	}
}

bool operator == (const string & s, const unsigned char* c)
{
	return s == reinterpret_cast<const char*>(c);
}