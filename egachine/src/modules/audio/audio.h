/*
 * Copyright (C) 2002 Jens Thiele <karme@berlios.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*!
   \file audio.h
   \brief Audio Interface
   \author Jens Thiele
*/

#ifndef AUDIO_H
#define AUDIO_H

#include "audioconfig.h"

//! audio interface class
/*!
  simple audio interface
  which is capable of playing one music file and a possible infinite
  number of samples - an implementation should allow at least 4 samples to be played
*/
class Audio
{
protected:
  //! audio configuration
  AudioConfig &m_sc;
public:
  typedef float R;
  // sample id
  typedef unsigned SID;
  static const SID INVALID_SID;

  // singleton pattern
  static Audio* audio;
  static bool init(AudioConfig &sc);
  static Audio* create(AudioConfig &sc);
  static bool deinit();
  
  // channel id
  typedef int CID;
  static const CID INVALID_CID=0;

  Audio(AudioConfig &sc) : m_sc(sc)
  {}
  virtual ~Audio()
  {}

  //! play music in the background
  /*!
    if another song is still playing it will be stopped
    (a nice implementation will fade out) and the new song ist started)

    \param data   (music data)
    \param dsize  (size of data)
    \param volume (0<=volume<=1 with 0=silent 1=maximum)
    \param repeat how often we should repeat this file (0=play only once -1=play endless)
  */
  virtual void playMusic(const char *data, unsigned dsize, R volume=R(1), int repeat=0)=0;
  
  virtual void stopMusic()=0;

  virtual bool playingMusic()=0;

  //! this is called every frame
  /*!
    This should allow an implementation without threads

    \param dt time elapsed since last call
  */
  virtual void step(R dt)=0;
  
  //! load a sample
  /*!
    \param uri the URI of the sample
  */
  virtual SID loadSample(const char *data, unsigned dsize)=0;
  
  //! play a previously loaded sample
  /*!
    \return the channel number the sample is played on
  */
  virtual CID playSample(SID id, int repeat=0)=0;

  //! modify channel settings
  /*!
    \param volume the volume setting (0<=volume<=1 with 0=silent 1=maximum)
    \param balance left right balance setting (-1<=balance<=1 with -1=left +1=right)
    \param pitch pitch factor - a simple implementation will only modify the speed
  */
  virtual void modifyChannel(CID channel, R volume, R balance=R(0), R pitch=R(1))=0;
  
  //! stop playing of sample
  virtual void stopChannel(CID channel)=0;

  virtual void unloadSample(SID id)=0;
};

#endif
