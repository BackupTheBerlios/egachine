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
   \file audiosdlmixer.cpp
   \brief SDL_mixer audio implementation
   \author Jens Thiele
*/

#include "audiosdlmixer.h"
#include <stdexcept>
#include <cassert>
#include "error.h"

// hack needed for c callback
AudioSDLMixer *thisptr=NULL;

static void music_finished()
{
  assert(thisptr);
  thisptr->musicFinished.emit();
}

AudioSDLMixer::AudioSDLMixer(AudioConfig &sc)
  : Audio(sc), audio_open(0), music(NULL), oldPlaying(false)
{
  assert(!thisptr);
  thisptr=this;
  
  if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 ) 
    throw std::runtime_error(std::string("Couldn't initialize SDL: ")+SDL_GetError());
    
  /* Open the audio device */
  int audio_rate = m_sc.srate;
  Uint16 audio_format=AUDIO_S8;
  if (m_sc.sbits==16)
    audio_format=AUDIO_S16;
  int audio_channels=1;
  if (m_sc.stereo) audio_channels=2;
  int audio_buffers = m_sc.sbuffers;

  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
    throw std::runtime_error(std::string("Couldn't open audio: ")+SDL_GetError());
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    /*    printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
	   (audio_format&0xFF),
	   (audio_channels > 1) ? "stereo" : "mono", 
	   audio_buffers ); */
  }
  audio_open = 1;
  
  /* Set the external music player, if any */
  // TODO: perhaps we should not use this for security reasons
  Mix_SetMusicCMD(getenv("MUSIC_CMD"));

  Mix_HookMusicFinished(music_finished);
}

AudioSDLMixer::~AudioSDLMixer()
{
  deinit();
}

void AudioSDLMixer::deinit()
{
  Mix_HookMusicFinished(NULL);
  thisptr=NULL;
  stopMusic();
  for (unsigned i=0;i<samples.size();++i)
    unloadSample(i);
  if ( audio_open ) {
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audio_open = 0;
  }
}

void
AudioSDLMixer::playMusic(const char *data, unsigned dsize, R volume, int repeat)
{
  if (music) Mix_FreeMusic(music);
  JGACHINE_CHECK(data);
  JGACHINE_CHECK(dsize>0);
  SDL_RWops* rw=SDL_RWFromConstMem(data,dsize);
  if (!rw) {
    JGACHINE_WARN(std::string("Couldn't play music:")+SDL_GetError());
    return;
  }
  music = Mix_LoadMUS_RW(rw);
  SDL_FreeRW(rw);
  if ( music == NULL ) {
    JGACHINE_WARN(std::string("Couldn't play music:")+SDL_GetError());
    return;
  }
  Mix_FadeInMusic(music,repeat,2000);
}
  
void
AudioSDLMixer::stopMusic() 
{
  if( Mix_PlayingMusic() ) {
    Mix_FadeOutMusic(1500);
    SDL_Delay(1500);
  }
  if ( music ) {
    Mix_FreeMusic(music);
    music = NULL;
  }
}
  
void 
AudioSDLMixer::step(R dt) 
{
  int playing=Mix_PlayingMusic();
  if ((!playing)&&(oldPlaying)) {
    JGACHINE_MSG("info","reached");
  }
}
  
Audio::SID
AudioSDLMixer::loadSample(const char *data, unsigned dsize) 
{
  JGACHINE_CHECK(data&&(dsize>0));
  Audio::SID sid=Audio::INVALID_SID;
  for (unsigned i=0;i<samples.size();++i)
    if (!samples[i]) sid=i;
  bool push=false;
  if (sid==INVALID_SID) {
    push=true;
    sid=samples.size();
    if (sid==INVALID_SID) {
      JGACHINE_WARN(std::string("Couldn't load sample:")+SDL_GetError());
      return sid;
    }
  }
  
  SDL_RWops* rw=SDL_RWFromConstMem(data,dsize);
  if (!rw) {
    JGACHINE_WARN(std::string("Couldn't load sample:")+SDL_GetError());
    return Audio::INVALID_SID;
  }

  // TODO: is it also freed on error?
  Mix_Chunk *r=Mix_LoadWAV_RW(rw, 1);
  if (!r) {
    JGACHINE_WARN(std::string("Couldn't load sample:")+SDL_GetError());
    return Audio::INVALID_SID;
  }
  if (push)
    samples.push_back(r);
  else
    samples[sid]=r;
  return sid;
}
  
Audio::CID
AudioSDLMixer::playSample(Audio::SID sid, int repeat)
{
  if ((sid==Audio::INVALID_SID)||(sid>=samples.size())||(!samples[sid])) {
    JGACHINE_WARN(std::string("Couldn't play sample:")<<sid<<"("<<SDL_GetError()<<")");
    return Audio::INVALID_CID;
  }
  return Mix_PlayChannel(-1,samples[sid],repeat);
}

void
AudioSDLMixer::modifyChannel(Audio::CID channel, R volume, R balance, R pitch)
{
  // todo balance?
  Mix_Volume(channel, int(volume*128));
}

void 
AudioSDLMixer::stopChannel(Audio::CID channel) 
{
  // todo
}

void 
AudioSDLMixer::unloadSample(Audio::SID sid) 
{
  if ((sid==Audio::INVALID_SID)||(sid>=samples.size())||(!samples[sid]))
    return;
  Mix_FreeChunk(samples[sid]);
  samples[sid]=NULL;
}
