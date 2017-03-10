#pragma once

#include "XmlNode.h"
#include "XmlNodeSet.h"
#include <libxml/parser.h>
#include "no_warning_map"
#include "no_warning_string"

namespace opencollada
{
	class XmlNode;

	using XPathCacheKey = std::tuple<xmlNodePtr, std::string>;
	using XPathCache = std::map<XPathCacheKey, XmlNodeSet>;

	class XmlDoc
	{
		friend class XmlSchema;
		friend class XmlNode;
		friend class ScopedSetDocRoot;

	public:
		XmlDoc() = default;
		XmlDoc(XmlDoc && other);
		virtual ~XmlDoc();

		const XmlDoc & operator = (XmlDoc && other);

		virtual void readFile(const std::string & path);

		operator bool() const;
		void reset();
		XmlNode root() const;

	private:
		XmlDoc(const XmlDoc&) = delete;
		const XmlDoc& operator = (const XmlDoc & other) = delete;

		static XmlDoc & GetXmlDoc(xmlDocPtr doc);

	private:
		xmlDocPtr mDoc = nullptr;

		XPathCache mXPathCache;
	};

	// Temporary change document root node. Original root node is restored when object is destroyed.
	// Used to validate some portion of the document.
	// /!\ New root node keeps links to its parent, next and previous nodes!
	// So don't use XPath queries while root is changed. /!\.
	class ScopedSetDocRoot
	{
	public:
		ScopedSetDocRoot(const XmlDoc & doc, const XmlNode & node);
		~ScopedSetDocRoot();

	private:
		XmlNode mDocOldChildren;
		XmlNode mDocOldLast;
	};
}