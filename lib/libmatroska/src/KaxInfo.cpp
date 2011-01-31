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
	\version \$Id: KaxInfo.cpp 1078 2005-03-03 13:13:04Z robux4 $
	\author Steve Lhomme     <robux4 @ users.sf.net>
*/
#include "matroska/KaxInfo.h"
#include "matroska/KaxInfoData.h"

#include "matroska/KaxContexts.h"
#include "matroska/KaxDefines.h"

// sub elements
START_LIBMATROSKA_NAMESPACE

DEFINE_START_SEMANTIC(KaxInfo)
DEFINE_SEMANTIC_ITEM(false, true, KaxSegmentUID)
DEFINE_SEMANTIC_ITEM(false, true, KaxSegmentFilename)
DEFINE_SEMANTIC_ITEM(false, true, KaxPrevUID)
DEFINE_SEMANTIC_ITEM(false, true, KaxPrevFilename)
DEFINE_SEMANTIC_ITEM(false, true, KaxNextUID)
DEFINE_SEMANTIC_ITEM(false, true, KaxNextFilename)
DEFINE_SEMANTIC_ITEM(false, false, KaxSegmentFamily)
DEFINE_SEMANTIC_ITEM(false, false, KaxChapterTranslate)
DEFINE_SEMANTIC_ITEM(true, true, KaxTimecodeScale)
DEFINE_SEMANTIC_ITEM(false, true, KaxDuration)
DEFINE_SEMANTIC_ITEM(false, true, KaxDateUTC)
DEFINE_SEMANTIC_ITEM(false, true, KaxTitle)
DEFINE_SEMANTIC_ITEM(true, true, KaxMuxingApp)
DEFINE_SEMANTIC_ITEM(true, true, KaxWritingApp)
DEFINE_END_SEMANTIC(KaxInfo)

DEFINE_MKX_MASTER   (KaxInfo,   0x1549A966, 4, KaxSegment, "Info");
DEFINE_MKX_UNISTRING(KaxMuxingApp,  0x4D80, 2, KaxInfo, "MuxingApp");
DEFINE_MKX_UNISTRING(KaxWritingApp, 0x5741, 2, KaxInfo, "WritingApp");

END_LIBMATROSKA_NAMESPACE
