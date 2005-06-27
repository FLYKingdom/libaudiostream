/*
Copyright � Grame 2002

This library is free software; you can redistribute it and modify it under
the terms of the GNU Library General Public License as published by the
Free Software Foundation version 2 of the License, or any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITYTAudioBuffer<float>*
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License
for more details.

You should have received a copy of the GNU Library General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
grame@rd.grame.fr

*/

//---------------------
// Class TAudioChannel
//---------------------

#include "TAudioChannel.h"
#include "TAudioGlobals.h"

#include "UAudioTools.h"
#include "TPanTable.h"

TAudioChannel::TAudioChannel()
{
    fStream = 0;
    fInserted = false;
    fPan = DEFAULT_PAN;
    fVol = DEFAULT_VOL;

    fLeftOut = 0;
    fRightOut = 1;

    fMixBuffer = new TLocalAudioBuffer<float>(TAudioGlobals::fBuffer_Size, TAudioGlobals::fOutput);
}

TAudioChannel::~TAudioChannel()
{
    fRendererStream.ClearStream(); // to avoid desallocation by destructor
    fFadeStream.ClearStream(); // to avoid desallocation by destructor
    delete fMixBuffer;
}

/*--------------------------------------------------------------------------*/
// External API
/*--------------------------------------------------------------------------*/

// Warning : SetStream replaces the internal stream with a new one, desallocation has to be done externally

void TAudioChannel::SetStream(TAudioStreamPtr stream)
{
    fRendererStream.SetStream(stream);
    fFadeStream.SetStream(stream, FADE_TIME1, FADE_TIME1);
    fStream = stream;
}

TAudioStreamPtr TAudioChannel::GetStream()
{
    return fStream;
}

void TAudioChannel::SoundOn()
{
    if (fStream && (fFadeStream.IsIdle()))
        fFadeStream.FadeIn();
}

// Synchonous stop : wait for the FadeOut to finish

void TAudioChannel::SoundOff()
{
    if (fStream && (!fFadeStream.IsIdle())) {
        fFadeStream.FadeOut();
        ChannelInfo info;

        // Wait until Fade Out ends
        do {
            GetInfo(&info);
            AudioSleep(50);
        } while (info.fStatus != TFadeAudioStream::kIdle);
    }
}

void TAudioChannel::Reset()
{
    if (fStream)
        fFadeStream.Reset();
}

void TAudioChannel::GetInfo(ChannelInfo* info)
{
    info->fStatus = fFadeStream.GetStatus();
    info->fCurFrame = fFadeStream.GetFrame();
    info->fVol = fVol;
    info->fPan = fPan;
    info->fLeftOut = fLeftOut;
    info->fRightOut = fRightOut;
}

/*--------------------------------------------------------------------------*/
// Internal API
/*--------------------------------------------------------------------------*/

bool TAudioChannel::Mix(TAudioBuffer<float>* dst, long framesNum, long channels)
{
    // Init buffer
    UAudioTools::ZeroFloatBlk(fMixBuffer->GetFrame(0), TAudioGlobals::fBuffer_Size, TAudioGlobals::fOutput);

    long res = fFadeStream.Read(fMixBuffer, framesNum, 0, channels);

    MY_FLOAT leftvol = TPanTable::GetVolLeft(short(fVol), short(fPan));
    MY_FLOAT rightvol = TPanTable::GetVolRight(short(fVol), short(fPan));

    UAudioTools::MixFrameToFrameBlk(dst->GetFrame(0), fMixBuffer->GetFrame(0), framesNum, channels, leftvol, rightvol);

    // Stops when the stream is empty
    return (res == framesNum);
}

