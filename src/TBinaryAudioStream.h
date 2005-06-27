/*
Copyright � Grame 2002

This library is free software; you can redistribute it and modify it under
the terms of the GNU Library General Public License as published by the
Free Software Foundation version 2 of the License, or any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License
for more details.

You should have received a copy of the GNU Library General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
grame@rd.grame.fr

*/

#ifndef __TBinaryAudioStream__
#define __TBinaryAudioStream__

#include "TAudioStream.h"

//--------------------------
// Class TBinaryAudioStream
//--------------------------
/*!
\brief  A TBinaryAudioStream contains two streams, not directly instanciated.
*/

class TBinaryAudioStream : public TDecoratedAudioStream
{

    protected:

        TAudioStreamPtr fStream1, fStream2;

    public:

        TBinaryAudioStream(TAudioStreamPtr s1, TAudioStreamPtr s2, TAudioStreamPtr init)
                : TDecoratedAudioStream(init), fStream1(s1), fStream2(s2)
        {}
        // To avoid desallocation but the base TDecoratedAudioStream class
        virtual ~TBinaryAudioStream()
        {
            fStream = NULL;
            delete fStream1;
            delete fStream2;
        }

        virtual long Read(TAudioBuffer<float>* buffer, long framesNum, long framePos, long channels) = 0;

        virtual void Reset()
        {
            fStream1->Reset();
            fStream2->Reset();
        }
        virtual void Stop()
        {
            fStream1->Stop();
            fStream2->Stop();
        }

        virtual TAudioStreamPtr CutBegin(long frames) = 0;
        virtual long Length() = 0;
        virtual long Channels() = 0;
        virtual TAudioStreamPtr Copy() = 0;

        TAudioStreamPtr getBranch1()
        {
            return fStream1;
        }
        TAudioStreamPtr getBranch2()
        {
            return fStream2;
        }

};

typedef TBinaryAudioStream * TBinaryAudioStreamPtr;

#endif