#include <math.h>
#include <unistd.h>
#include <string.h>

#include "kc/config.h"
#include "kc/system.h"

#if HAVE_LIBSDL

#include <SDL/SDL.h>

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/ctc.h"
#include "kc/sound.h"

static double _sound_counter = 0;
static double _sound_freq = 1000;
static double _sound_init = 0;

static long long _idx = 0;

#define SOUND_BUFFER_SIZE (8192)
#define SOUND_SAMPLE_FREQ (11025)
#define SYNC_COUNTER (10)

void
Sound::sound_callback(void *userdata, unsigned char *stream, int len)
{
  int b;
  static int idx = SYNC_COUNTER;
  static int val = 0;
  static double x = -1;
  static double inc = (1750000.0 / SOUND_SAMPLE_FREQ);
  static long long sum_counter = 0;
  static long long old_counter = 0;
  static bool first = true;
  
  Sound *self = (Sound *)userdata;
  sndop *op = NULL;
  if (self->_sndop_list.size() > 0)
    {
      op = (*(self->_sndop_list.begin()));
      if (op->_counter == 0)
	{
	  _sound_counter = _sound_init;
	  printf("init - %12.2f\n", _sound_counter);
	}
    }

#if 0
  if ((z80->getCounter() - _sound_counter) < 1300000)
    {
      _sound_counter = z80->getCounter() - 1300000;
      printf("catching up...\n");
    }
  if ((z80->getCounter() - _sound_counter) > 2600000)
    {
      _sound_counter = z80->getCounter() - 2600000;
      printf("skipping sycles...\n");
    }
#endif

  //printf("+ %10Ld - %12.2f - %12.2f\n", z80->getCounter(), _sound_counter, z80->getCounter() - _sound_counter);

  for (b = 0;b < len;b++)
    {
      _sound_counter += inc;

      if (x < 0)
	{
	  x += (SOUND_SAMPLE_FREQ / _sound_freq);
	  val = -val;
	}
      if (val == 0)
	stream[b] = 0x80;
      else
	stream[b] = val;
      x--;
      _idx++;

      if (op != NULL)
	{
	  if (_sound_counter > op->_counter)
	    {
	      double f = op->_freq;
	      long long c = op->_counter;
	      delete op;
	      self->_sndop_list.pop_front();
	      printf("* %Ld - %.2f (%Ld) => new sound op, freq = %.2f\n", z80->getCounter(), _sound_counter, c, f);
	      printf("* cnt: %10Ld - %12.2f - %d\n", _idx, _sound_freq, val);
	      _idx = 0;

	      if (f == 0)
		{
		  val = 0;
		  //self->_playing = false;
		}
	      else
		{
		  if (val == 0)
		    val = 120;
		  _sound_freq = f;
		}
	      
	      op = NULL;
	      if (self->_sndop_list.size() > 0)
		op = (*(self->_sndop_list.begin()));
	    }
	}
    }
  
#if 0
  if (first)
    {
      first = false;
    }
  else
    {
      sum_counter += z80->getCounter() - old_counter;
      if (--idx == 0)
	{
	  double xxx = (double)(sum_counter / SYNC_COUNTER) / SOUND_BUFFER_SIZE;
	  
	  idx = SYNC_COUNTER;
	  printf("%Ld - %.3f - %.3f = %Ld - %.2f\n",
		 sum_counter / SYNC_COUNTER, inc, xxx,
		 z80->getCounter(), _sound_counter);
	  inc += (xxx - inc) / 3.0;
	  sum_counter = 0;
	}
    }
  old_counter = z80->getCounter();
  printf("- %Ld - %.2f\n", z80->getCounter(), _sound_counter);
#endif
}

Sound::Sound(void)
{
  int ret;
  SDL_AudioSpec wanted, obtained;
  
  wanted.freq = SOUND_SAMPLE_FREQ;
  wanted.format = AUDIO_S8;
  wanted.channels = 1;
  wanted.samples = SOUND_BUFFER_SIZE;
  wanted.callback = sound_callback;
  wanted.userdata = this;

  ret = SDL_OpenAudio(&wanted, &obtained);
  printf("ret = %d\n", ret);

  printf("obtained.freq     = %d\n", obtained.freq);
  printf("obtained.format   = %d\n", obtained.format);
  printf("obtained.channels = %d\n", obtained.channels);
  printf("obtained.samples  = %d\n", obtained.samples);
  printf("obtained.silence  = %d\n", obtained.silence);
  printf("obtained.size     = %d\n", obtained.size);

  pio->register_callback_B_out(this);
  ctc->register_callback(0, this);

  SDL_PauseAudio(1);
  _playing = false;
  _dummy_sndop = new sndop(0, 0);
  _last_sndop = _dummy_sndop;
}

Sound::~Sound(void)
{
  delete _dummy_sndop;
}

void
Sound::start()
{
  double freq = (1750000.0 / 2) / ctc->getTimerValue(0);

  if (!_playing)
    {
      _playing = true;
      SDL_PauseAudio(0);

      _sound_init = z80->getCounter();
      printf("Sound::start() - %10Ld - %12.2f\n", z80->getCounter(), freq);
      if (_last_sndop->_freq != freq)
	{
	  _last_sndop = new sndop(0, freq);
	  _sndop_list.push_back(_last_sndop);
	}
    }
}

void
Sound::stop()
{
  if (_playing)
    {
      //_playing = false;
      //SDL_PauseAudio(1);
      printf("Sound::stop()  - %10Ld - %12.2f\n", z80->getCounter(), 0);
      if (_last_sndop->_freq != 0)
	{
	  _last_sndop = new sndop(z80->getCounter(), 0);
	  _sndop_list.push_back(_last_sndop);
	}
    }
}

void
Sound::callback_B_out(byte_t val)
{
  if (((val & 1) == 1) && ((val & 0x0e) != 0x0e) && !ctc->isReset(0))
    start();
  else
    stop();
}

void
Sound::ctc_callback_start(int channel)
{
  if (ctc->getTimerValue(channel) == 0)
    return;

  printf("Sound::ctc_callback_start()\n");
  start();
}

void
Sound::ctc_callback_stop(int channel)
{
  stop();
}

void
Sound::ctc_callback_TC(int channel, long tc)
{
  double freq;

  switch (channel)
    {
    case 0:
      freq = (1750000.0 / 2) / tc;
      printf("Sound::cb_TC() - %10Ld - %12.2f\n", z80->getCounter(), freq);
      if (_last_sndop->_freq != freq)
	{
	  _last_sndop = new sndop(z80->getCounter(), freq);
	  _sndop_list.push_back(_last_sndop);
	}
      break;
    default:
      break;
    }
}

#endif /* HAVE_LIBSDL */
