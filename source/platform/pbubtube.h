/*=========================================================================

	pbubtube.h

	Author:		CRB
	Created: 
	Project:	Spongebob
	Purpose: 

	Copyright (c) 2001 Climax Development Ltd

===========================================================================*/

#ifndef __PLATFORM_PBUBTUBE_H__
#define __PLATFORM_PBUBTUBE_H__

#ifndef __PLATFORM_PLATFORM_H__
#include "platform\platform.h"
#endif

class CNpcBubbleTubePlatform : public CNpcPlatform
{
public:
	virtual void		postInit();
	virtual void		setWaypoints( sThingPlatform *ThisPlatform );
protected:
	virtual void		collidedWith(CThing *_thisThing);
	virtual void		processMovement( int _frames );
	virtual void		processTimer( int _frames );

	u8					m_isActivated;
	u8					m_returning;
	s32					m_maxExtension;
};

#endif