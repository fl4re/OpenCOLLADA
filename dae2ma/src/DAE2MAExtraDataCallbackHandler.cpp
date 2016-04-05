/*
    Copyright (c) 2008-2009 NetAllied Systems GmbH

    This file is part of DAE2MA.

    Licensed under the MIT Open Source License, 
    for details please see LICENSE file or the website
    http://www.opensource.org/licenses/mit-license.php
*/

#include "DAE2MAStableHeaders.h"
#include "DAE2MAExtraDataCallbackHandler.h"

#include "generated14/COLLADASaxFWLColladaParserAutoGen14Attributes.h"


namespace DAE2MA
{

    //------------------------------
    ExtraDataCallbackHandler::ExtraDataCallbackHandler() 
        : mIsOriginalIdField (false)
	{
	}
	
    //------------------------------
	ExtraDataCallbackHandler::~ExtraDataCallbackHandler()
	{
	}

    //------------------------------
    const ExtraInfo* ExtraDataCallbackHandler::findExtraInfo ( 
        const COLLADAFW::UniqueId& uniqueId, 
        const StringHash& hashElement ) const
    {
        ExtraInfosMap::const_iterator it = mExtraInfos.find ( uniqueId );
        if ( it != mExtraInfos.end () )
        {
            const std::vector<ExtraInfo>& extraInfos = it->second;
            size_t numInfos = extraInfos.size ();
            for ( size_t i=0; i<numInfos; ++i )
            {
                if ( extraInfos[i].getElementHash () == hashElement )
                    return &extraInfos[i];
            }
        }
        return 0;
    }

    //------------------------------
    bool ExtraDataCallbackHandler::parseElement ( 
        const GeneratedSaxParser::ParserChar* profileName, 
        const StringHash& elementHash, 
        const COLLADAFW::UniqueId& uniqueId,
		COLLADAFW::Object* object ) 
    {
        if ( COLLADABU::Utils::equals ( PROFILE_MAYA, profileName ) ) 
        {
            mCurrentExtraInfo.setElementHash ( elementHash );
            mCurrentExtraInfo.setUniqueId ( uniqueId );
            return true;
        }

        return false;
    }

    //------------------------------
    bool ExtraDataCallbackHandler::elementBegin ( const GeneratedSaxParser::ParserChar* elementName, const GeneratedSaxParser::xmlChar** attributes ) 
    {
        if ( COLLADABU::Utils::equals ( PARAMETER_MAYA_ID, String (elementName) ) )
        {
            mIsOriginalIdField = true;
        }

        return true;
    }

    //------------------------------
    bool ExtraDataCallbackHandler::elementEnd ( const GeneratedSaxParser::ParserChar* elementName ) 
    {
        if ( mIsOriginalIdField )
        {
            mIsOriginalIdField = false;
            mExtraInfos [mCurrentExtraInfo.getUniqueId ()].push_back ( mCurrentExtraInfo );
            mCurrentExtraInfo.setOriginalMayaId ( EMPTY_STRING );
        }

        return true;
    }

    //------------------------------
    bool ExtraDataCallbackHandler::textData ( const GeneratedSaxParser::ParserChar* text, size_t textLength ) 
    {
        if ( mIsOriginalIdField )
        {
            mCurrentExtraInfo.setOriginalMayaId ( text, textLength );
        }
        return true;
    }






	//------------------------------
	ExtraCameraDataCallbackHandler::ExtraCameraDataCallbackHandler()
		: mIsVerticalAperture(false), mIsHorizontalAperture(false), mIsFocalLength(false)
	{
	}

	//------------------------------
	ExtraCameraDataCallbackHandler::~ExtraCameraDataCallbackHandler()
	{
	}


	bool ExtraCameraDataCallbackHandler::elementBegin(const GeneratedSaxParser::ParserChar* elementName, const GeneratedSaxParser::xmlChar** attributes)
	{
		if (COLLADABU::Utils::equals(PARAMETER_MAYA_VAPERTURE_PARAMETER, String(elementName)))
		{
			mIsVerticalAperture = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_HAPERTURE_PARAMETER, String(elementName)))
		{
			mIsHorizontalAperture = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_FOCAL_LENGTH_PARAMETER, String(elementName)))
		{
			mIsFocalLength = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_FILM_FIT_PARAMETER, String(elementName)))
		{
			mFilmFit = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_FILM_FIT_OFFSET_PARAMETER, String(elementName)))
		{
			mFilmFitOffset = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_FILM_OFFSET_X_PARAMETER, String(elementName)))
		{
			mFilmOffsetX = true;
		}
		else if (COLLADABU::Utils::equals(PARAMETER_MAYA_FILM_OFFSET_Y_PARAMETER, String(elementName)))
		{
			mFilmOffsetY = true;
		}

		return true;
	}

	bool ExtraCameraDataCallbackHandler::elementEnd(const GeneratedSaxParser::ParserChar* elementName)
	{

		std::map<COLLADAFW::UniqueId, std::vector<ExtraInfo> >::iterator it;

		if (mIsVerticalAperture || mIsHorizontalAperture || mIsFocalLength || mFilmFit || mFilmFitOffset || mFilmOffsetX || mFilmOffsetY)
		{
			it = mExtraInfos.find(mCurrentExtraInfo.getUniqueId());
			if (it == mExtraInfos.end())
				mExtraInfos[mCurrentExtraInfo.getUniqueId()].push_back(mCurrentExtraInfo);
		}

		if (mIsVerticalAperture)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getVerticalAperture() = mCurrentExtraInfo.getVerticalAperture();
			mIsVerticalAperture = false;
			mCurrentExtraInfo.setVerticalAperture(EMPTY_STRING);
		}
		else if (mIsHorizontalAperture)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getHorizontalAperture() = mCurrentExtraInfo.getHorizontalAperture();
			mIsHorizontalAperture = false;
			mCurrentExtraInfo.setHorizontalAperture(EMPTY_STRING);
		}
		else if (mIsFocalLength)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getFocalLength() = mCurrentExtraInfo.getFocalLength();
			mIsFocalLength = false;
			mCurrentExtraInfo.setFocalLength(EMPTY_STRING);
		}
		else if (mFilmFit)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getFilmFit() = mCurrentExtraInfo.getFilmFit();
			mFilmFit = false;
			mCurrentExtraInfo.setFilmFit(EMPTY_STRING);
		}
		else if (mFilmFitOffset)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getFilmFitOffset() = mCurrentExtraInfo.getFilmFitOffset();
			mFilmFitOffset = false;
			mCurrentExtraInfo.setFilmFitOffset(EMPTY_STRING);
		}
		else if (mFilmOffsetX)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getFilmOffsetX() = mCurrentExtraInfo.getFilmOffsetX();
			mFilmOffsetX = false;
			mCurrentExtraInfo.setFilmOffsetX(EMPTY_STRING);
		}
		else if (mFilmOffsetY)
		{
			mExtraInfos[mCurrentExtraInfo.getUniqueId()][0].getFilmOffsetY() = mCurrentExtraInfo.getFilmOffsetY();
			mFilmOffsetY = false;
			mCurrentExtraInfo.setFilmOffsetY(EMPTY_STRING);
		}

		return true;
	}

	bool ExtraCameraDataCallbackHandler::textData(const GeneratedSaxParser::ParserChar* text, size_t textLength)
	{

		if (mIsVerticalAperture)
			mCurrentExtraInfo.setVerticalAperture(text, textLength);
		else if (mIsHorizontalAperture)
			mCurrentExtraInfo.setHorizontalAperture(text, textLength);
		else if (mIsFocalLength)
			mCurrentExtraInfo.setFocalLength(text, textLength);
		else if (mFilmFit)
			mCurrentExtraInfo.setFilmFit(text, textLength);
		else if (mFilmFitOffset)
			mCurrentExtraInfo.setFilmFitOffset(text, textLength);
		else if (mFilmOffsetX)
			mCurrentExtraInfo.setFilmOffsetX(text, textLength);
		else if (mFilmOffsetY)
			mCurrentExtraInfo.setFilmOffsetY(text, textLength);

		return true;
	}
	

} // namespace DAE2MA
