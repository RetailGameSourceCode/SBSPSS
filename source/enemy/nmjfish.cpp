/*=========================================================================

	nmjfish.cpp

	Author:		CRB
	Created: 
	Project:	Spongebob
	Purpose: 

	Copyright (c) 2000 Climax Development Ltd

===========================================================================*/

#ifndef __ENEMY_NPC_H__
#include "enemy\npc.h"
#endif

#ifndef	__ENEMY_NMJFISH_H__
#include "enemy\nmjfish.h"
#endif

#ifndef __ENEMY_NPROJJF_H__
#include "enemy\nprojjf.h"
#endif

#ifndef __VID_HEADER_
#include "system\vid.h"
#endif

#ifndef	__UTILS_HEADER__
#include	"utils\utils.h"
#endif

#ifndef __GAME_GAME_H__
#include	"game\game.h"
#endif

#ifndef	__PLAYER_PLAYER_H__
#include	"player\player.h"
#endif

#ifndef __PROJECTL_PROJECTL_H__
#include "projectl\projectl.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::postInit()
{
	m_state = MOTHER_JELLYFISH_CYCLE;
	m_spawnTimer = 0;
	m_meterOn=false;

	if ( CLevel::getIsBossRespawn() )
	{
		m_health = CLevel::getBossHealth();
	}

	legsPos[0].vx = 80;
	legsPos[0].vy = -5;

	legsPos[1].vx = 40;
	legsPos[1].vy = 0;

	legsPos[2].vx = -40;
	legsPos[2].vy = -5;

	legsPos[3].vx = -80;
	legsPos[3].vy = 0;

	for ( int i = 0 ; i < 4 ; i++ )
	{
		legs[i] = (CFXJellyFishLegs*)CFX::Create(CFX::FX_TYPE_JELLYFISH_LEGS,this);
		legs[i]->Setup( legsPos[i].vx, legsPos[i].vy, i > 1 );
	}

	m_RGB = MJ_PINK;

	targetPos = Pos;

	m_movementTimer = GameState::getOneSecondInFrames() * 5;
	m_pulsateTimer = GameState::getOneSecondInFrames();

	m_renderScale = 2048 + ( ( ( 4096 - 2048 ) * m_health ) / m_data[m_type].initHealth );
	m_speed = m_data[m_type].speed + ( ( 3 * ( m_data[m_type].initHealth - m_health ) ) / m_data[m_type].initHealth );
	m_pauseTimer = m_maxPauseTimer = ( GameState::getOneSecondInFrames() * m_health ) / m_data[m_type].initHealth;
	m_invulnerableTimer = 0;

	m_attackCounter = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::setupWaypoints( sThingActor *ThisActor )
{
	u16	*PntList=(u16*)MakePtr(ThisActor,sizeof(sThingActor));

	u16 newXPos, newYPos;

	s32 startX = 0;

	m_npcPath.setWaypointCount( ThisActor->PointCount - 1 );

	newXPos = (u16) *PntList;
	setWaypointPtr( PntList );
	PntList++;
	newYPos = (u16) *PntList;
	PntList++;

	setStartPos( newXPos, newYPos );

	startX = ( newXPos << 4 ) + 8;

	if ( ThisActor->PointCount > 1 )
	{
		for (int pointNum = 1 ; pointNum < ThisActor->PointCount ; pointNum++ )
		{
			newXPos = (u16) *PntList;
			PntList++;
			newYPos = (u16) *PntList;
			PntList++;

			if ( pointNum == 1 )
			{
				setHeading( newXPos, newYPos );
			}

			if ( pointNum == ThisActor->PointCount - 2 )
			{
				startX = ( newXPos << 4 ) + 8;
				m_base.vx = startX;
				m_base.vy = ( newYPos << 4 ) + 16;
			}
			else if ( pointNum == ThisActor->PointCount - 1 )
			{
				m_cycleWidth = abs( startX - ( ( newXPos << 4 ) + 8 ) );
				m_halfCycleWidth = m_cycleWidth >> 1;
				m_base.vx += m_halfCycleWidth;
			}
		}
	}

	s32 minX, maxX, minY, maxY;

	m_npcPath.getPathXExtents( &minX, &maxX );
	m_npcPath.getPathYExtents( &minY, &maxY );

	m_thinkArea.x1 = minX;
	m_thinkArea.x2 = maxX;
	m_thinkArea.y1 = minY;
	m_thinkArea.y2 = maxY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::processMovement( int _frames )
{
	if ( m_invulnerableTimer > 0 )
	{
		m_invulnerableTimer -= _frames;
	}

	switch( m_state )
	{
		case MOTHER_JELLYFISH_CYCLE:
		{
			if ( m_movementTimer <= 0 )
			{
				if ( m_pulsateTimer <= 0 )
				{
					if ( m_pauseTimer <= 0 )
					{
						// fire at player

						s16 heading = ratan2( playerYDist, playerXDist ) & 4095;

						CProjectile *projectile;
						projectile = CProjectile::Create();
						DVECTOR newPos = Pos;
						projectile->init( newPos, heading );
						projectile->setGraphic( FRM__LIGHTNING1 );

						m_movementTimer = GameState::getOneSecondInFrames() * 5;
						m_pulsateTimer = GameState::getOneSecondInFrames();
						m_pauseTimer = m_maxPauseTimer;

						m_RGB = MJ_PINK;

						m_attackCounter++;

						if ( m_attackCounter > 2 )
						{
							m_attackCounter = 0;
							m_state = MOTHER_JELLYFISH_STRAFE_START;

							s32 minX, maxX, minY, maxY;

							m_npcPath.getPathXExtents( &minX, &maxX );
							m_npcPath.getPathYExtents( &minY, &maxY );

							targetPos.vx = minX;
							targetPos.vy = minY;
						}
					}
					else
					{
						if ( m_RGB == MJ_PINK )
						{
							m_RGB = MJ_WHITE;
						}
						else
						{
							m_RGB = MJ_PINK;
						}

						m_pauseTimer -= _frames;
					}
				}
				else
				{
					m_pulsateTimer -= _frames;

					m_renderScale = 2048 + ( ( ( 4096 - 2048 ) * m_health ) / m_data[m_type].initHealth );
					m_renderScale += ( ( 256 * rsin( ( ( m_pulsateTimer << 14 ) / GameState::getOneSecondInFrames() ) & 4095 ) ) >> 12 );
				}
			}
			else
			{
				m_movementTimer -= _frames;

				s32 distX, distY;

				distX = targetPos.vx - Pos.vx;
				distY = targetPos.vy - Pos.vy;

				if( abs( distX ) < 10 && abs( distY ) < 10 )
				{
					s32 minX, maxX, minY, maxY;

					m_npcPath.getPathXExtents( &minX, &maxX );
					m_npcPath.getPathYExtents( &minY, &maxY );

					targetPos.vx = minX + ( getRnd() % ( maxX - minX + 1 ) );
					targetPos.vy = minY + ( getRnd() % ( maxY - minY + 1 ) );
				}
				else
				{
					processGenericGotoTarget( _frames, distX, distY, m_speed );
				}
			}

			break;
		}

		case MOTHER_JELLYFISH_BEGIN_CIRCLE:
		{
			s32 distX, distY;

			distX = playerXDist + 70;
			distY = playerYDist;

			if ( abs( distX ) > 10 || abs( distY ) > 10 )
			{
				processGenericGotoTarget( _frames, distX, distY, m_speed );
			}
			else
			{
				m_state = MOTHER_JELLYFISH_CIRCLE;
				m_extension = 0;
			}

			break;
		}

		case MOTHER_JELLYFISH_CIRCLE:
		{
			m_extension += 64 * _frames;

			if ( m_extension < 3072 )
			{
				CPlayer *player = GameScene.getPlayer();

				DVECTOR playerPos = player->getPos();

				Pos.vx = playerPos.vx + ( ( 70 * rcos( m_extension ) ) >> 12 );
				Pos.vy = playerPos.vy + ( ( 70 * rsin( m_extension ) ) >> 12 );
			}
			else
			{
				m_state = MOTHER_JELLYFISH_EXIT;
			}

			break;
		}

		case MOTHER_JELLYFISH_EXIT:
		{
			Pos.vx += 8 * _frames;

			DVECTOR	offset = CLevel::getCameraPos();

			if ( Pos.vx - offset.vx > VidGetScrW() )
			{
				m_isActive = false;
				setToShutdown();
				CGameScene::setBossHasBeenKilled();
			}

			break;
		}

		case MOTHER_JELLYFISH_STRAFE_START:
		{
			s32 distX, distY;

			distX = targetPos.vx - Pos.vx;
			distY = targetPos.vy - Pos.vy;

			if( abs( distX ) < 10 && abs( distY ) < 10 )
			{
				s32 minX, maxX, minY, maxY;

				m_npcPath.getPathXExtents( &minX, &maxX );
				m_npcPath.getPathYExtents( &minY, &maxY );

				targetPos.vx = maxX;
				targetPos.vy = minY;

				m_state = MOTHER_JELLYFISH_STRAFE;
			}
			else
			{
				processGenericGotoTarget( _frames, distX, distY, m_speed );
			}

			break;
		}

		case MOTHER_JELLYFISH_STRAFE:
		{
			if ( m_RGB == MJ_PINK )
			{
				m_RGB = MJ_WHITE;
			}
			else
			{
				m_RGB = MJ_PINK;
			}

			s32 distX, distY;

			distX = targetPos.vx - Pos.vx;
			distY = targetPos.vy - Pos.vy;

			if( abs( distX ) < 10 && abs( distY ) < 10 )
			{
				s32 minX, maxX, minY, maxY;

				m_npcPath.getPathXExtents( &minX, &maxX );
				m_npcPath.getPathYExtents( &minY, &maxY );

				targetPos.vx = minX + ( getRnd() % ( maxX - minX + 1 ) );
				targetPos.vy = minY + ( getRnd() % ( maxY - minY + 1 ) );

				m_state = MOTHER_JELLYFISH_CYCLE;
				m_RGB = MJ_PINK;
			}
			else
			{
				processGenericGotoTarget( _frames, distX, distY, m_speed );
			}

			break;
		}
	}



	/*s32 xDist, yDist;
	s32 xDistSqr, yDistSqr;

	switch( m_state )
	{
		case MOTHER_JELLYFISH_RETURN_TO_START_1:
		case MOTHER_JELLYFISH_RETURN_TO_START_2:
		case MOTHER_JELLYFISH_RETURN_TO_START_3:
		{
			xDist = m_base.vx - this->Pos.vx - m_halfCycleWidth;
			xDistSqr = xDist * xDist;
			yDist = m_base.vy - this->Pos.vy;
			yDistSqr = yDist * yDist;

			if ( xDistSqr + yDistSqr > 100 )
			{
				processGenericGotoTarget( _frames, xDist, yDist, m_speed );
			}
			else
			{
				// have arrived at base position

				m_movementTimer = GameState::getOneSecondInFrames() * 10;
				m_state++;
				m_extension = -m_halfCycleWidth;
				m_extendDir = EXTEND_RIGHT;
			}

			break;
		}

		case MOTHER_JELLYFISH_CYCLE_1:
		case MOTHER_JELLYFISH_CYCLE_2:
		case MOTHER_JELLYFISH_CYCLE_3:
		{
			m_movementTimer -= _frames;

			s32 xExtension;

			if ( m_extendDir == EXTEND_RIGHT )
			{
				if ( m_extension < m_halfCycleWidth )
				{
					m_extension += 3 * _frames;

					xExtension = ( m_halfCycleWidth * rsin( ( m_extension << 10 ) / m_halfCycleWidth ) ) >> 12;

					Pos.vx = m_base.vx + xExtension;
					Pos.vy = m_base.vy - ( ( 50 * rsin( ( xExtension << 12 ) / m_cycleWidth ) ) >> 12 );

					m_heading = 0;
				}
				else
				{
					m_extendDir = EXTEND_LEFT;

					if ( m_movementTimer < 0 )
					{
						m_controlFunc = NPC_CONTROL_CLOSE;
						m_state++;
						m_jellyfishCount = 3;
					}
				}
			}
			else
			{
				if ( m_extension > -m_halfCycleWidth )
				{
					m_extension -= 3 * _frames;

					xExtension = ( m_halfCycleWidth * rsin( ( m_extension << 10 ) / m_halfCycleWidth ) ) >> 12;

					Pos.vx = m_base.vx + xExtension;
					Pos.vy = m_base.vy + ( ( 50 * rsin( ( xExtension << 12 ) / m_cycleWidth ) ) >> 12 );

					m_heading = 2048;
				}
				else
				{
					m_extendDir = EXTEND_RIGHT;

					if ( m_movementTimer < 0 )
					{
						m_controlFunc = NPC_CONTROL_CLOSE;
						m_state++;
						m_jellyfishCount = 3;
					}
				}
			}

			break;
		}

		default:
			break;
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::processClose( int _frames )
{
	/*switch( m_state )
	{
		case MOTHER_JELLYFISH_ATTACK_PLAYER_SHOCK:
		{
			// seek position above user

			CPlayer *player = GameScene.getPlayer();
			DVECTOR playerPos = player->getPos();
			DVECTOR seekPos;
			s32 xDist, yDist;
			s32 xDistSqr, yDistSqr;

			seekPos = playerPos;
			seekPos.vy -= 100;

			xDist = seekPos.vx - this->Pos.vx;
			xDistSqr = xDist * xDist;
			yDist = seekPos.vy - this->Pos.vy;
			yDistSqr = yDist * yDist;

			if ( xDistSqr + yDistSqr > 400 )
			{
				processGenericGotoTarget( _frames, xDist, yDist, m_speed );
			}
			else
			{
				// fire at user

				m_controlFunc = NPC_CONTROL_MOVEMENT;
				m_state = MOTHER_JELLYFISH_RETURN_TO_START_1;

				CSoundMediator::playSfx( CSoundMediator::SFX_JELLYFISH_ATTACK );
			}

			break;
		}

		default:
		{
			s32 xExtension;

			if ( m_extendDir == EXTEND_RIGHT )
			{
				if ( m_extension < m_halfCycleWidth )
				{
					m_extension += 3 * _frames;

					xExtension = ( m_halfCycleWidth * rsin( ( m_extension << 10 ) / m_halfCycleWidth ) ) >> 12;

					Pos.vx = m_base.vx + xExtension;
					Pos.vy = m_base.vy - ( ( 50 * rcos( ( xExtension << 11 ) / m_cycleWidth ) ) >> 12 );

					m_heading = 0;

					//spawnJellyfish( _frames );
				}
				else
				{
					m_controlFunc = NPC_CONTROL_MOVEMENT;
					m_state++;
				}
			}
			else
			{
				if ( m_extension > -m_halfCycleWidth )
				{
					m_extension -= 3 * _frames;

					xExtension = ( m_halfCycleWidth * rsin( ( m_extension << 10 ) / m_halfCycleWidth ) ) >> 12;

					Pos.vx = m_base.vx + xExtension;
					Pos.vy = m_base.vy + ( ( 50 * rcos( ( xExtension << 11 ) / m_cycleWidth ) ) >> 12 );

					m_heading = 2048;

					//spawnJellyfish( _frames );
				}
				else
				{
					m_controlFunc = NPC_CONTROL_MOVEMENT;
					m_state++;
				}
			}

			break;
		}
	}*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::shutdown()
{
	if ( m_isActive )
	{
		CLevel::setIsBossRespawn( true );
		CLevel::setBossHealth( m_health );
	}

	CNpcEnemy::shutdown();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*void CNpcMotherJellyfishEnemy::spawnJellyfish( int _frames )
{
	if ( m_jellyfishCount )
	{
		if ( m_spawnTimer > 0 )
		{
			m_spawnTimer -= _frames;
		}
		else
		{
			CNpcEnemy *enemy;
			enemy = CNpcEnemy::Create( NPC_PROJECTILE_JELLYFISH );
			enemy->setStartPos( Pos.vx >> 4, ( Pos.vy + 20 ) >> 4 );

			enemy->setWaypointCount( m_npcPath.getWaypointCount() );
			enemy->setWaypointPtr( m_npcPath.getWaypointPtr() );

			enemy->setPathType( CNpcPath::PONG_PATH );
			enemy->postInit();

			m_jellyfishCount--;

			m_spawnTimer = 1 * GameState::getOneSecondInFrames();
		}
	}
}*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::render()
{
	SprFrame = NULL;

	if ( m_isActive )
	{
		CEnemyThing::render();

		if (canRender())
		{
			if (!m_meterOn)
			{
				CFXNRGBar	*T=(CFXNRGBar*)CFX::Create(CFX::FX_TYPE_NRG_BAR,this);
				T->SetMax(m_health);
				m_meterOn=true;
			}

			DVECTOR &renderPos=getRenderPos();

			SprFrame = m_actorGfx->Render(renderPos,m_animNo,( m_frame >> 8 ),false);
			m_actorGfx->RotateScale( SprFrame, renderPos, 0, m_renderScale, m_renderScale );

			setRGB0( SprFrame, ( m_RGB & 255 ), ( ( m_RGB >> 8 ) & 255 ), ( ( m_RGB >> 16 ) & 255 ) );

			sBBox boundingBox = m_actorGfx->GetBBox();
			setCollisionSize( ( boundingBox.XMax - boundingBox.XMin ), ( boundingBox.YMax - boundingBox.YMin ) );
			setCollisionCentreOffset( ( boundingBox.XMax + boundingBox.XMin ) >> 1, ( boundingBox.YMax + boundingBox.YMin ) >> 1 );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*void CNpcMotherJellyfishEnemy::processUserCollision( CThing *thisThing )
{
}*/

void CNpcMotherJellyfishEnemy::processShot( int _frames )
{
	s16 scale;
	scale = 2048 + ( ( ( 4096 - 2048 ) * m_health ) / m_data[m_type].initHealth );

	for ( int i = 0 ; i < 4 ; i++ )
	{
		legs[i]->Setup( ( legsPos[i].vx * scale ) >> 12, legsPos[i].vy, i > 1 );
		legs[i]->setScale( scale );
	}

	switch ( m_state )
	{
		case NPC_GENERIC_HIT_CHECK_HEALTH:
		{
			if ( m_health > 0 )
			{
				m_health -= 5;

				m_renderScale = 2048 + ( ( ( 4096 - 2048 ) * m_health ) / m_data[m_type].initHealth );
				m_speed = m_data[m_type].speed + ( ( 3 * ( m_data[m_type].initHealth - m_health ) ) / m_data[m_type].initHealth );
				m_maxPauseTimer = ( GameState::getOneSecondInFrames() * m_health ) / m_data[m_type].initHealth;

				m_state = MOTHER_JELLYFISH_CYCLE;
			}
			else
			{
				m_state = MOTHER_JELLYFISH_BEGIN_CIRCLE;
			}

			m_controlFunc = NPC_CONTROL_MOVEMENT;

			m_invulnerableTimer = 2 * GameState::getOneSecondInFrames();

			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNpcMotherJellyfishEnemy::collidedWith(CThing *_thisThing)
{
	if ( m_state == MOTHER_JELLYFISH_CYCLE )
	{
		if ( m_isActive && !m_isCaught && !m_isDying )
		{
			switch(_thisThing->getThingType())
			{
				case TYPE_PLAYER:
				{
					CPlayer *player = (CPlayer *) _thisThing;

					ATTACK_STATE playerState = player->getAttackState();

					if(playerState==ATTACK_STATE__NONE)
					{
						if ( !player->isRecoveringFromHit() )
						{
							switch( m_data[m_type].detectCollision )
							{
								case DETECT_NO_COLLISION:
								{
									// ignore

									break;
								}

								case DETECT_ALL_COLLISION:
								{
									m_oldControlFunc = m_controlFunc;
									m_controlFunc = NPC_CONTROL_COLLISION;

									processUserCollision( _thisThing );

									break;
								}

								case DETECT_ATTACK_COLLISION_GENERIC:
								{
									processAttackCollision();
									processUserCollision( _thisThing );

									break;
								}
							}
						}
					}
					else if ( m_invulnerableTimer <= 0 )
					{
						// player is attacking, respond appropriately

						if ( m_controlFunc != NPC_CONTROL_SHOT )
						{
							if(playerState==ATTACK_STATE__BUTT_BOUNCE)
							{
								player->justButtBouncedABadGuy();
							}
							m_controlFunc = NPC_CONTROL_SHOT;
							m_state = NPC_GENERIC_HIT_CHECK_HEALTH;

							drawAttackEffect();
						}
					}

					break;
				}

				case TYPE_ENEMY:
				{
					CNpcEnemy *enemy = (CNpcEnemy *) _thisThing;

					if ( canCollideWithEnemy() && enemy->canCollideWithEnemy() )
					{
						processEnemyCollision( _thisThing );
					}

					break;
				}

				default:
					ASSERT(0);
					break;
			}
		}
	}
}
