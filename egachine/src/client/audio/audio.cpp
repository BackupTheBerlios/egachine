#include "audio.h"
#include "sdlmixer/audiosdlmixer.h"
#include <stdexcept>
#include <iostream>
#include <limits>

const Audio::SID Audio::INVALID_SID=std::numeric_limits<Audio::SID>::max();
Audio* Audio::audio=NULL;

Audio* 
Audio::create(AudioConfig &sc)
{
  if (sc.sdriver=="sdlmixer") {
    // todo - I am not sure if this is correct
    AudioSDLMixer* n=NULL;
    try{
      n=new AudioSDLMixer(sc);
    }catch(const std::runtime_error &error){
      std::cerr << error.what() << "\naudio disabled\n";
      delete n;
      n=NULL;
    }
    return n;
  }
  return NULL;
}

bool
Audio::init(AudioConfig &sc)
{
  Audio::audio=create(sc);
  return Audio::audio!=NULL;
}

bool
Audio::deinit()
{
  if (!Audio::audio) return true;
  delete Audio::audio;
  Audio::audio=NULL;
  return true;
}


