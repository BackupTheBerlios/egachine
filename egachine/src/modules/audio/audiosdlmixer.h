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
   \file audiosdlmixer.h
   \brief mikmod audio implementation
   \author Jens Thiele
*/

#ifndef AUDIOSDLMIXER_H
#define AUDIOSDLMIXER_H

#include "audio.h"
#include <SDL.h>
#define USE_RWOPS
#include <SDL_mixer.h>
#undef USE_RWOPS
#include <vector>

//! audio implementation using SDL_mixer
class AudioSDLMixer : public Audio
{
protected:
  void deinit();

  int audio_open;
  Mix_Music *music;
  bool oldPlaying;
  
  typedef std::vector<Mix_Chunk *> Samples;
  Samples samples;

public:
  
  AudioSDLMixer(AudioConfig &sc);
  
  ~AudioSDLMixer();

  void playMusic(const char *data, unsigned dsize, R volume=R(1), int repeat=0);
  void stopMusic();
  bool playingMusic();
  
  void step(R dt);
  
  SID loadSample(const char *data, unsigned dsize);
  CID playSample(SID id, int repeat=0);
  void modifyChannel(CID, R volume, R balance=R(0), R pitch=R(1));
  void stopChannel(CID);
  void unloadSample(SID id);
};

#endif
