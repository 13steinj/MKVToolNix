/****************************************************************************
** libmatroska : parse Matroska files, see http://www.matroska.org/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libmatroska.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** See http://www.matroska.org/license/lgpl/ for LGPL licensing information.**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
	\file
	\version \$Id: KaxInfoData.cpp 353 2010-06-27 05:24:02Z robux4 $
	\author Steve Lhomme     <robux4 @ users.sf.net>
	\author John Cannon      <spyder2555 @ users.sf.net>
*/
#include "matroska/KaxInfoData.h"
#include "matroska/KaxContexts.h"
#include "matroska/KaxDefines.h"

START_LIBMATROSKA_NAMESPACE

DEFINE_START_SEMANTIC(KaxChapterTranslate)
DEFINE_SEMANTIC_ITEM(false, false, KaxChapterTranslateEditionUID)
DEFINE_SEMANTIC_ITEM(true, true, KaxChapterTranslateCodec)
DEFINE_SEMANTIC_ITEM(true, true, KaxChapterTranslateID)
DEFINE_END_SEMANTIC(KaxChapterTranslate)

DEFINE_MKX_BINARY       (KaxSegmentUID,                 0x73A4, 2, KaxInfo, "SegmentUID");
DEFINE_MKX_UNISTRING    (KaxSegmentFilename,            0x7384, 2, KaxInfo, "SegmentFilename");
DEFINE_MKX_BINARY_CONS  (KaxPrevUID,                  0x3CB923, 3, KaxInfo, "PrevUID");
DEFINE_MKX_UNISTRING    (KaxPrevFilename,             0x3C83AB, 3, KaxInfo, "PrevFilename");
DEFINE_MKX_BINARY_CONS  (KaxNextUID,                  0x3EB923, 3, KaxInfo, "NextUID");
DEFINE_MKX_UNISTRING    (KaxNextFilename,             0x3E83BB, 3, KaxInfo, "NextFilename");
DEFINE_MKX_BINARY       (KaxSegmentFamily,              0x4444, 2, KaxInfo, "SegmentFamily");
DEFINE_MKX_MASTER       (KaxChapterTranslate,           0x6924, 2, KaxInfo, "ChapterTranslate");
DEFINE_MKX_UINTEGER     (KaxChapterTranslateEditionUID, 0x69FC, 2, KaxChapterTranslate, "ChapterTranslateEditionUID");
DEFINE_MKX_UINTEGER     (KaxChapterTranslateCodec,      0x69BF, 2, KaxChapterTranslate, "ChapterTranslateCodec");
DEFINE_MKX_BINARY       (KaxChapterTranslateID,         0x69A5, 2, KaxChapterTranslate, "ChapterTranslateID");
DEFINE_MKX_UINTEGER_DEF(KaxTimecodeScale,            0x2AD7B1, 3, KaxInfo, "TimecodeScale", 1000000);
DEFINE_MKX_FLOAT        (KaxDuration,                   0x4489, 2, KaxInfo, "Duration");
DEFINE_MKX_DATE         (KaxDateUTC,                    0x4461, 2, KaxInfo, "DateUTC");
DEFINE_MKX_UNISTRING    (KaxTitle,                      0x7BA9, 2, KaxInfo, "Title");

KaxPrevUID::KaxPrevUID(EBML_EXTRA_DEF)
:KaxSegmentUID(EBML_DEF_BINARY_CTX(KaxPrevUID) EBML_DEF_SEP EBML_EXTRA_CALL)
{
}

KaxNextUID::KaxNextUID(EBML_EXTRA_DEF)
:KaxSegmentUID(EBML_DEF_BINARY_CTX(KaxNextUID) EBML_DEF_SEP EBML_EXTRA_CALL)
{
}

#if defined(HAVE_EBML2) || defined(HAS_EBML2)
KaxSegmentUID::KaxSegmentUID(EBML_DEF_CONS EBML_DEF_SEP EBML_EXTRA_DEF)
:EbmlBinary(EBML_DEF_PARAM EBML_DEF_SEP EBML_EXTRA_CALL)
{
}
#endif

END_LIBMATROSKA_NAMESPACE
