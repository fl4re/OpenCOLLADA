/*
    Copyright (c) 2008-2009 NetAllied Systems GmbH

    This file is part of DAE2MA.

    Licensed under the MIT Open Source License, 
    for details please see LICENSE file or the website
    http://www.opensource.org/licenses/mit-license.php
*/

#ifndef __DAE2MA_EXTRADATACALLBACKHANDLER_H__
#define __DAE2MA_EXTRADATACALLBACKHANDLER_H__

#include "DAE2MAPrerequisites.h"

#include "COLLADASaxFWLIExtraDataCallbackHandler.h"


namespace DAE2MA
{

    class ExtraInfo
    { 
    private:

        /** The hash value of the currently parsed extra data element. */
        StringHash mElementHash; 

        /** The uniqueId of the currently parsed extra data element. */
        COLLADAFW::UniqueId mUniqueId;

        /** The text value of the current original maya id. */
        String mOriginalMayaId;
		String mVerticalAperture;
		String mHorizontalAperture;
		String mFocalLength;
		String mFilmFit;
		String mFilmFitOffset;
		String mFilmOffsetX;
		String mFilmOffsetY;

    public:

        /** Constructor. */
        ExtraInfo () {}

        /** Destructor. */
        virtual ~ExtraInfo () {}

        /** The hash value of the currently parsed extra data element. */
        const StringHash& getElementHash () const { return mElementHash; }
        void setElementHash ( const StringHash& val ) { mElementHash = val; }

        /** The uniqueId of the currently parsed extra data element. */
        const COLLADAFW::UniqueId& getUniqueId () const { return mUniqueId; }
        void setUniqueId ( const COLLADAFW::UniqueId& val ) { mUniqueId = val; }

        /** The text value of the current original maya id. */
        const String& getOriginalMayaId () const { return mOriginalMayaId; }
        void setOriginalMayaId ( const String& val ) { mOriginalMayaId = val; }
        void setOriginalMayaId ( const GeneratedSaxParser::ParserChar* text, size_t textLength ) 
        { 
            mOriginalMayaId.assign ( text, textLength ); 
        }

		/** The text value of the current original maya id. */
		String& getVerticalAperture() { return mVerticalAperture; }
		const String& getVerticalAperture() const { return mVerticalAperture; }
		void setVerticalAperture(const String& val) { mVerticalAperture = val; }
		void setVerticalAperture(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mVerticalAperture.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getHorizontalAperture() { return mHorizontalAperture; }
		const String& getHorizontalAperture() const { return mHorizontalAperture; }
		void setHorizontalAperture(const String& val) { mHorizontalAperture = val; }
		void setHorizontalAperture(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mHorizontalAperture.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getFocalLength() { return mFocalLength; }
		const String& getFocalLength() const { return mFocalLength; }
		void setFocalLength(const String& val) { mFocalLength = val; }
		void setFocalLength(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mFocalLength.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getFilmFit() { return mFilmFit; }
		const String& getFilmFit() const { return mFilmFit; }
		void setFilmFit(const String& val) { mFilmFit = val; }
		void setFilmFit(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mFilmFit.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getFilmFitOffset() { return mFilmFitOffset; }
		const String& getFilmFitOffset() const { return mFilmFitOffset; }
		void setFilmFitOffset(const String& val) { mFilmFitOffset = val; }
		void setFilmFitOffset(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mFilmFitOffset.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getFilmOffsetX() { return mFilmOffsetX; }
		const String& getFilmOffsetX() const { return mFilmOffsetX; }
		void setFilmOffsetX(const String& val) { mFilmOffsetX = val; }
		void setFilmOffsetX(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mFilmOffsetX.assign(text, textLength);
		}

		/** The text value of the current original maya id. */
		String& getFilmOffsetY() { return mFilmOffsetY; }
		const String& getFilmOffsetY() const { return mFilmOffsetY; }
		void setFilmOffsetY(const String& val) { mFilmOffsetY = val; }
		void setFilmOffsetY(const GeneratedSaxParser::ParserChar* text, size_t textLength)
		{
			mFilmOffsetY.assign(text, textLength);
		}

    };

    /** Implementation of an extra data callback handler with the callback handler interface. */
	class ExtraDataCallbackHandler : public COLLADASaxFWL::IExtraDataCallbackHandler
    {
    private:

        typedef std::map<COLLADAFW::UniqueId, std::vector<ExtraInfo> > ExtraInfosMap;

	//private:
	protected:
        /** True, if the current text field is the original id field. */
        bool mIsOriginalIdField;

        ExtraInfo mCurrentExtraInfo;

        ExtraInfosMap mExtraInfos;

	public:

        /** Constructor. */
		ExtraDataCallbackHandler();

        /** Destructor. */
		virtual ~ExtraDataCallbackHandler();

        /** Returns the extra info with the searched id and hash string value. */
        const ExtraInfo* findExtraInfo ( 
            const COLLADAFW::UniqueId& uniqueId, 
            const StringHash& hashElement ) const;

        /** Method to ask, if the current callback handler want to read the data of the given extra element. */
        virtual bool parseElement ( 
            const GeneratedSaxParser::ParserChar* profileName, 
            const StringHash& elementHash, 
            const COLLADAFW::UniqueId& uniqueId,
			COLLADAFW::Object* object);

        /** The methods to get the extra data tags to the registered callback handlers. */
        virtual bool elementBegin( const GeneratedSaxParser::ParserChar* elementName, const GeneratedSaxParser::xmlChar** attributes);
        virtual bool elementEnd(const GeneratedSaxParser::ParserChar* elementName );
        virtual bool textData(const GeneratedSaxParser::ParserChar* text, size_t textLength);

	private:

        /** Disable default copy ctor. */
		ExtraDataCallbackHandler( const ExtraDataCallbackHandler& pre );

        /** Disable default assignment operator. */
		const ExtraDataCallbackHandler& operator= ( const ExtraDataCallbackHandler& pre );

	};

	class ExtraCameraDataCallbackHandler : public ExtraDataCallbackHandler
	{

	public:

		/** Constructor. */
		ExtraCameraDataCallbackHandler();

		/** Destructor. */
		virtual ~ExtraCameraDataCallbackHandler();

		/** True, if the current text field is the original id field. */
		bool mIsVerticalAperture;
		bool mIsHorizontalAperture;
		bool mIsFocalLength;
		int mFilmFit;
		float mFilmFitOffset;
		float mFilmOffsetX;
		float mFilmOffsetY;

	public:

		/** The methods to get the extra data tags to the registered callback handlers. */
		virtual bool elementBegin(const GeneratedSaxParser::ParserChar* elementName, const GeneratedSaxParser::xmlChar** attributes);
		virtual bool elementEnd(const GeneratedSaxParser::ParserChar* elementName);
		virtual bool textData(const GeneratedSaxParser::ParserChar* text, size_t textLength);
	};


} // namespace DAE2MA

#endif // __DAE2MA_EXTRADATACALLBACKHANDLER_H__
