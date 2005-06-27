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

#ifndef __TAudioStream__
#define __TAudioStream__

#include "TAudioConstants.h"
#include "UAudioTools.h"
#include "TAudioBuffer.h"

#include <stdio.h>

//--------------------
// Class TAudioStream
//--------------------
/*!
\brief The base class for all streams.
*/ 
/*
The Stop method may be used by subclasses to implement clean stream stopping (for example, flushing the disk for
disk based stream, closing socket for a network based stream...etc..)
Calling the Stop method when necessary is the reponsability of the object that uses the stream.
 
Be carefull : Closing may be done by another thread and no synchronization is done between the Stop method 
and the possible other thread.
*/

class TAudioStream
{

    public:

        TAudioStream()
        {}
        virtual ~TAudioStream()
        {}

        virtual long Write(TAudioBuffer<float>* buffer, long framesNum, long framePos, long channels)
        {
            return 0;
        }
        virtual long Read(TAudioBuffer<float>* buffer, long framesNum, long framePos, long channels)
        {
            return 0;
        }

        // Reset the stream to it's beginning
        virtual void Reset()
        {}

        // Stop the stream
        virtual void Stop()
        {}

        // Cut the beginning of the stream
        virtual TAudioStream* CutBegin(long frames)
        {
            return 0;
        }

        // Length in frames
        virtual long Length()
        {
            return 0;
        }

        // Number of channels
        virtual long Channels()
        {
            return 0;
        }

        // Copy the structure
        virtual TAudioStream* Copy()
        {
            return 0;
        }
};

typedef TAudioStream * TAudioStreamPtr;

//-------------------------
// Class TUnaryAudioStream
//-------------------------
/*!
\brief The base class for all unary streams.
*/

class TUnaryAudioStream
{

    protected:

        TAudioStreamPtr fStream; // Decorated stream

    public:

        TAudioStreamPtr getBranch1()
        {
            return fStream;
        }
};

typedef TUnaryAudioStream * TUnaryAudioStreamPtr;


//-----------------------------
// Class TDecoratedAudioStream
//-----------------------------
/*!
\brief  A TDecoratedAudioStream object decorates the contained stream.
*/

class TDecoratedAudioStream : public TAudioStream, public TUnaryAudioStream
{

    public:

        TDecoratedAudioStream(): TAudioStream()
        {
            fStream = 0;
        }
        TDecoratedAudioStream(TAudioStreamPtr stream): TAudioStream()
        {
            fStream = stream;
        }

        virtual ~TDecoratedAudioStream()
        {
            delete fStream;
        }

        virtual long Write(TAudioBuffer<float>* buffer, long framesNum, long framePos, long channels)
        {
            assert(fStream);
            return fStream->Write(buffer, framesNum, framePos, channels);
        }

        virtual long Read(TAudioBuffer<float>* buffer, long framesNum, long framePos, long channels)
        {
            assert(fStream);
            return fStream->Read(buffer, framesNum, framePos, channels);
        }

        virtual void Reset()
        {
            assert(fStream);
            fStream->Reset();
        }

        virtual void Stop()
        {
            assert(fStream);
            fStream->Stop();
        }

        virtual TAudioStreamPtr CutBegin(long frames)
        {
            assert(fStream);
            return fStream->CutBegin(frames);
        }

        virtual long Length()
        {
            assert(fStream);
            return fStream->Length();
        }

        virtual long Channels()
        {
            assert(fStream);
            return fStream->Channels();
        }

        virtual TAudioStreamPtr Copy()
        {
            return new TDecoratedAudioStream(fStream->Copy());
        }
};

typedef TDecoratedAudioStream * TDecoratedAudioStreamPtr;

#endif