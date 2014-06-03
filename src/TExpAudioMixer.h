/*

Copyright (C) Grame 2002-2014

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
research@grame.fr

*/

#ifndef __TExpAudioMixer__
#define __TExpAudioMixer__

#include "TAudioClient.h"
#include "TAudioChannel.h"
#include "TAudioGlobals.h"
#include "TAudioDate.h"
#include "TFaustAudioEffect.h"
#include "la_smartpointer.h"

#include <list>
#include <map>

//----------------
// Class TCommand
//----------------

struct TCommand : public la_smartable1 {

        SymbolicDate fStartDate;
  
        inline audio_frames_t GetDate(map<SymbolicDate, audio_frames_t>& date_map, SymbolicDate date)
        {
            if (date_map.find(date) == date_map.end()) {
                date_map[date] = date->getDate();
            }
            return date_map[date];
        }
        
        bool InBuffer(audio_frames_t date, audio_frames_t cur_frame, long frames)
        {
            return (date >= cur_frame && date < cur_frame + frames);
        }
        
        audio_frames_t GetDate() { return fStartDate->GetDate(); }
        
        TCommand() 
        {}
        TCommand(SymbolicDate date):fStartDate(date)
        {}
        virtual ~TCommand() 
        {}
         
        virtual bool Execute(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                            map<SymbolicDate, audio_frames_t>& date_map, 
                            audio_frames_t cur_frame, 
                            long frames) = 0;
                            
        bool operator< (const TCommand& command) const
        {
            return fStartDate->getDate() < command.fStartDate->getDate();
        }
        
        virtual long GetOffset(audio_frames_t cur_frame, long frames) { return -1; }
        
};

typedef LA_SMARTP<TCommand> TCommandPtr;


//--------------------------------------------
// Class TControlCommand : a control command
//--------------------------------------------

struct TControlCommand : public TCommand {
   
        TControlCommand()
        {}
        TControlCommand(SymbolicDate date):TCommand(date)
        {}
        virtual ~TControlCommand() 
        {}
         
        /* 
            Returns the offset in buffer, or frames if not in buffer.
        */
        virtual long GetOffset(audio_frames_t cur_frame, long frames) 
        { 
            if (InBuffer(fStartDate->getDate(), cur_frame, frames)) {
                return fStartDate->getDate() - cur_frame;
            } else {
                return frames;
            }
        }

};

typedef LA_SMARTP<TControlCommand> TControlCommandPtr;

//---------------------------------------------------------------------------
// Class TEffectControlCommand : a command to set Faust effect control value
//---------------------------------------------------------------------------

struct TEffectControlCommand : public TControlCommand {
    
        TAudioEffectInterfacePtr fEffect;
        string fPath;
        float fValue;
    
        TEffectControlCommand() 
        {}
        TEffectControlCommand(TAudioEffectInterfacePtr effect, const string& path, float value, SymbolicDate date) 
            : TControlCommand(date), fEffect(effect), fPath(path), fValue(value)
        {}
        virtual ~TEffectControlCommand() 
        {}
         
        bool Execute(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                    map<SymbolicDate, audio_frames_t>& date_map, 
                    audio_frames_t cur_frame, 
                    long frames)
        {
            if (InBuffer(fStartDate->getDate(), cur_frame, frames)) {
                //printf("SetControlValue %s %f\n", fPath.c_str(), fValue);
                fEffect->SetControlValue(fPath.c_str(), fValue);
                return false;
            } else {
                return true;
            }
        }
    
};

//-------------------------------------------------------------------------
// Class TExternalControlCommand : a command to call an user given callback
//-------------------------------------------------------------------------

typedef void (*AudioControlCallback) (audio_frames_t date, float value, void *arg);

struct TExternalControlCommand : public TCommand {

        AudioControlCallback fCallback;
        void* fArg;
        float fValue;
    
        TExternalControlCommand(AudioControlCallback callback, float value, void* arg)
            :fCallback(callback), fArg(arg), fValue(value)
        {}
        virtual ~TExternalControlCommand() 
        {}
         
        bool Execute(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                    map<SymbolicDate, audio_frames_t>& date_map, 
                    audio_frames_t cur_frame, 
                    long frames)
        {
            if (InBuffer(fStartDate->getDate(), cur_frame, frames)) {
                fCallback(fStartDate->getDate(), fValue, fArg); 
                return false;
            } else {
                return true;
            }
        }

};

//---------------------------------------------------------
// Class TStreamCommand : a command to start/stop streams
//---------------------------------------------------------
    
struct TStreamCommand : public TCommand {
        
        TRTRendererAudioStreamPtr fStream; // SmartPtr here...
            
        SymbolicDate fStopDate;
 
        TStreamCommand(TRTRendererAudioStreamPtr stream, SymbolicDate start_date, SymbolicDate stop_date)
                :TCommand(start_date), fStream(stream), fStopDate(stop_date)
        {}
        virtual ~TStreamCommand() 
        {}
        
        void SetSartDate(SymbolicDate start_date) { fStartDate = start_date; }
        void SetStopDate(SymbolicDate stop_date) { fStopDate = stop_date; }
         
        /*
        bool Execute(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                    map<SymbolicDate, audio_frames_t>& date_map, 
                    audio_frames_t cur_frame, 
                    long frames)
        {
            // Keeps the same value for the entire audio cycle
            audio_frames_t start_date = GetDate(date_map, fStartDate);
            audio_frames_t stop_date = GetDate(date_map, fStopDate);
            
            printf("TStreamCommand::Execute start_date = %lld stop_date = %lld cur_frame = %lld frames = %ld\n", start_date, stop_date, cur_frame, frames);
            
            long start_offset = 0;
            long stop_offset = std::abs(long(cur_frame - stop_date));
            long frame_num = std::min(frames, stop_offset);
            bool to_play = false;
            long res = 0;
            
            if (InBuffer(start_date, cur_frame, frames)) {
                // New stream to play...
                start_offset = start_date - cur_frame;
                to_play = true;
                printf("Start stream fCurFrame = %lld offset = %ld\n", cur_frame, start_offset);
            } else if (cur_frame > start_date) {
                // Stream currently playing...
                to_play = true;
            }
            
            printf("TStreamCommand::Execute frame_num = %d start_offset = %d\n", frame_num, start_offset);
            
            // Play it...
            if (to_play && (((res = fStream->Read(&shared_buffer, frame_num, start_offset)) < frames))) {
                // End of stream
                //printf("Stop stream frame_num = %ld res = %ld\n", frame_num, res);
                return false;
            } else {
                return true;
            }
        }
        */
        
        bool Execute(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                    map<SymbolicDate, audio_frames_t>& date_map, 
                    audio_frames_t cur_frame, 
                    long frames)
        {
            // Keeps the same value for the entire audio cycle
            audio_frames_t start_date = GetDate(date_map, fStartDate);
            audio_frames_t stop_date = GetDate(date_map, fStopDate);
            
            //printf("TStreamCommand::Execute start_date = %lld stop_date = %lld cur_frame = %lld frames = %ld\n", start_date, stop_date, cur_frame, frames);
            
            // Possibly entire buffer to play
            long start_offset = 0;
            long stop_offset = frames;  
            
            // Init values
            long frame_num;
            bool to_play = false;
            bool to_stop = false;
            long res = 0;
            
            // Possibly move start_offset inside this buffer
            if (InBuffer(start_date, cur_frame, frames)) {
                // New stream to play...
                start_offset = start_date - cur_frame;
                to_play = true;
                //printf("Start stream fCurFrame = %lld start_offset = %ld\n", cur_frame, start_offset);
            } else if (cur_frame > start_date) {
                // Stream currently playing...
                to_play = true;
            }
            
            // Possibly move stop_offset inside this buffer
            if ((stop_date < cur_frame) || InBuffer(stop_date, cur_frame, frames)) {
                // Stream will be stopped in this buffer...
                stop_offset = stop_date - cur_frame;
                to_stop = true;
                //printf("Stop stream fCurFrame = %lld stop_offset = %ld\n", cur_frame, stop_offset);
            }
            
            // Then compute effective frame number to play
            frame_num = stop_offset - start_offset;
            
            //printf("TStreamCommand::Execute frame_num = %d start_offset = %d\n", frame_num, start_offset);
            
            // Play it...
            if (to_play) {
                if (to_stop || (res = fStream->Read(&shared_buffer, frame_num, start_offset)) < frame_num) {
                    // End of stream
                    //printf("Stop stream frame_num = %ld res = %ld\n", frame_num, res);
                    return false;
                }
            }
            
            return true;
        }

};

typedef LA_SMARTP<TStreamCommand> TStreamCommandPtr;

//--------------------
// Class TCommandList
//--------------------

class TCommandList : public list<TCommandPtr> 
{

    private:
    
        static bool compare_command_date (TCommandPtr first, TCommandPtr second)
        {
            return first->GetDate() < second->GetDate();
        }
    
        volatile bool fNeedSort;
    
    public:
    
        TCommandList():fNeedSort(false)
        {}
        virtual ~TCommandList() {}
    
        void AddCommand(TCommandPtr command)
        { 
            push_back(command);
            fNeedSort = true;
        }

        void RemoveCommand(TCommandPtr command) 
        { 
            remove(command);
            fNeedSort = true;
        }
        
        void PossiblySort()
        {
            if (fNeedSort) {
                sort(compare_command_date); 
                fNeedSort = false;
                //printf("SORT\n");
            }
        }
        
        void NeedSort()
        {
            fNeedSort = true;
        }
        
        void Print() 
        {
            TCommandList::iterator it;
            for (it = begin(); it != end(); it++) {
                TCommandPtr command = *it;
                printf("Command date %lld\n", command->GetDate());
            }
        }

};

typedef TCommandList COMMANDS;
typedef TCommandList::iterator COMMANDS_ITERATOR;

//----------------------
// Class TExpAudioMixer
//----------------------

class TExpAudioMixer : public TAudioClient
{

    private:
    
        COMMANDS fStreamCommands;     // List of stream commands
        COMMANDS fControlCommands;    // List of control commands
        
        audio_frames_t fCurFrame;
   
        bool AudioCallback(float** inputs, float** outputs, long frames);
        
        
        void ExecuteControlSlice(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                                        map<SymbolicDate, audio_frames_t>& date_map, 
                                        audio_frames_t cur_frame, 
                                        long offset,
                                        long slice);

        void ExecuteStreamsSlice(TSharedNonInterleavedAudioBuffer<float>& shared_buffer, 
                                map<SymbolicDate, audio_frames_t>& date_map, 
                                audio_frames_t cur_frame, 
                                long offset,
                                long slice);

        long GetNextControlOffset(audio_frames_t cur_frame, long frames);
      
    public:

        TExpAudioMixer():fCurFrame(0) {}
        virtual ~TExpAudioMixer() {}
        
        void AddStreamCommand(TCommandPtr command)
        { 
            fStreamCommands.AddCommand(command);
        }
        void RemoveStreamCommand(TCommandPtr command) 
        { 
            fStreamCommands.RemoveCommand(command);
        }
        
        void AddControlCommand(TCommandPtr command)
        { 
            fControlCommands.AddCommand(command);
        }
        void RemoveControlCommand(TCommandPtr command) 
        { 
            fControlCommands.RemoveCommand(command);
        }
      
        TStreamCommandPtr GetStreamCommand(TAudioStreamPtr stream);
        
        int GetCommandSize() { return fStreamCommands.size(); }
        
        void NeedSort()
        {
            fControlCommands.NeedSort();
            fStreamCommands.NeedSort();
        }
    
};

typedef TExpAudioMixer * TExpAudioMixerPtr;

#endif


