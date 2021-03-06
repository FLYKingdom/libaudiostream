/*

Copyright (C) Grame 2002-2013

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

#ifndef __LibAudioStream__
#define __LibAudioStream__

/*!
\addtogroup interface LibAudioStream programming interface

@{
*/

#ifdef __cplusplus
extern "C"
{
#endif

#define NO_ERR 0
#define OPEN_ERR -1
#define CLOSE_ERR -1
#define LOAD_ERR -3
#define FILE_NOT_FOUND_ERR -4

    enum {kPlayingChannel = 0, kIdleChannel};
    enum {kPortAudioRenderer = 0, kJackRenderer, kCoreAudioRenderer};

    /*!
    \brief Sound channel info.
    */
    typedef struct ChannelInfo* ChannelInfoPtr;
    typedef struct ChannelInfo {
        long fStatus;  // 1 = playing , 0 = idle
        long fCurFrame;
        float fVol;
        float fPanLeft;
		float fPanRight;
        long fLeftOut;
        long fRightOut;
    } ChannelInfo;
	
	/*!
    \brief Audio device info.
    */
	typedef struct DeviceInfo* DeviceInfoPtr;
	typedef struct DeviceInfo {
		char fName[64];      
		long fMaxInputChannels;
		long fMaxOutputChannels; 
		long fDefaultBufferSize; 
		double fDefaultSampleRate;
	} DeviceInfo;
    
    /*!
    \brief Renderer state.
    */
    typedef struct RendererInfo* RendererInfoPtr;
    typedef struct RendererInfo {
        long fInput;   				// Number of input channels
        long fOutput;   			// Number of output channels
        long fSampleRate; 			// Sampling Rate
        long fBufferSize;			// I/O Buffer size
        uint64_t fCurFrame;			// Currrent sample
        uint64_t fCurUsec;			// Current microsecond
        long fOutputLatencyFrame;	// Output latency in frames
        long fOutputLatencyUsec;	// Output latency in microsecond
        long fInputLatencyFrame;	// Input latency in frames
        long fInputLatencyUsec;		// Input latency in microsecond
    } RendererInfo;

    // Opaque pointers
    typedef void* AudioPlayerPtr;
	typedef void* AudioRendererPtr;
 	typedef void* AudioStreamPtr;
	typedef void* AudioClientPtr;
	
    typedef void* AudioEffectPtr;
    typedef void* AudioEffectListPtr;
	typedef void* AudioEffectInterfacePtr;
	
	typedef void (*StopCallback)(void* context);

    /*!
    \brief Gives the library version number.
    \return the library version number as a 3 digits long value.
    */
	long LibVersion();
	
    /**
     * @defgroup SoundFunctions Sound creation and manipulation functions
     * @{
     */
 
	/*!
    \brief Create a stream that will produce "silence".
    \param lengthFrame The number of null frame to be produced.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeNullSoundPtr(long lengthFrame);
    /*!
    \brief Create a file reader stream.
    \param name The sound file pathname.
    \return A pointer to new stream object or NULL if the file cannot be opened.
    */
    AudioStreamPtr MakeReadSoundPtr(char* name);
    /*!
    \brief Create a file region reader stream. 
    \param name The sound file pathname.
    \param beginFrame The start frame of the region.
    \param endFrame The end frame of the region.
    \return A pointer to new stream object or NULL if the wanted region is not part of the file.
    */
    AudioStreamPtr MakeRegionSoundPtr(char* name, long beginFrame, long endFrame);
	/*!
    \brief Transform a stream in a stereo stream.
    \param sound The stream to be transformed.
    \return A pointer to new stream object.
    */
	AudioStreamPtr MakeStereoSoundPtr(AudioStreamPtr sound);
    /*!
    \brief Create a fade on a stream.
    \param sound The stream to be "faded".
    \param fadeIn The fadein length in frames.
    \param fadeOut The fadeout length in frames.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeFadeSoundPtr(AudioStreamPtr sound, long fadeIn, long fadeOut);
    /*!
    \brief Loop a stream.
    \param sound The stream to be looped.
    \param num The number of loops.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeLoopSoundPtr(AudioStreamPtr sound, long num);
    /*!
    \brief Cut in a stream : the portion between 0 and beginFrame will be removed, the portion between endFrame and the stream end will be removed.
    \param sound The stream to be cutted.
    \param beginFrame The start frame number of the stream part to remove.
    \param endFrame The end frame number of the stream part to remove
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeCutSoundPtr(AudioStreamPtr sound, long beginFrame, long endFrame);
    /*!
    \brief Put two streams in sequence.
    \param s1 The first stream in the sequence.
    \param s2 The second stream in the sequence.
    \param crossFade A crossface section expressed in frames.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeSeqSoundPtr(AudioStreamPtr s1, AudioStreamPtr s2, long crossFade);
    /*!
    \brief Mix two streams.
    \param s1 The first stream in the mix.
    \param s2 The second stream in the mix.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeMixSoundPtr(AudioStreamPtr s1, AudioStreamPtr s2);
    /*!
    \brief Apply a list of effects on a stream.
    \param sound The stream to be transformed.
    \param effect_list The effect list to be used.
    \param fadeIn A fadein section frames before the effect is fully applied.
    \param fadeOut A fadeout section frames before the effect is fully removed.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeTransformSoundPtr(AudioStreamPtr sound, AudioEffectListPtr effect_list, long fadeIn, long fadeOut);
	/*!
    \brief To pitchshift or timestretch a stream.
    \param sound The stream to be transformed.
    \param pitch_shift The address of a double value to be used as pitch_shift.
    \param time_strech The address of a double value to be used as time_strech.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakePitchSchiftTimeStretchSoundPtr(AudioStreamPtr sound, double* pitch_shift, double* time_strech);
    /*!
    \brief Create a stream writer.
    \param name The sound file pathname.
    \param sound The stream to be saved.
    \param format A libsndfile format.
    \return A pointer to new stream object or NULL if the file cannot be opened.
    */
    AudioStreamPtr MakeWriteSoundPtr(char* name, AudioStreamPtr sound, long format);
    /*!
    \brief Create an inputstream.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeInputSoundPtr();
    /*!
    \brief Create an renderer "wrapper" on a stream, to be used for direct access to the stream content.
    \return A pointer to new stream object.
    */
    AudioStreamPtr MakeRendererSoundPtr(AudioStreamPtr sound);
    /*!
    \brief Get the stream length in frames.
    \param sound The stream.
    \return The stream length in frames.
    */
    long GetLengthSoundPtr(AudioStreamPtr sound);
    /*!
    \brief Get the stream number of channels.
    \param sound The stream.
    \return The number of channels.
    */
    long GetChannelsSoundPtr(AudioStreamPtr sound);
    /*!
    \brief Read a buffer of the stream.
    \param sound The stream.
    \param buffer A buffer to be filled with frames.
    \param buffer_size The buffer length.
    \param channels The number of channels in the buffer.
    \return The number of read frames.
    */
    long ReadSoundPtr(AudioStreamPtr sound, float* buffer, long buffer_size, long channels);
    /*!
    \brief Delete a stream.
    \param sound The stream to be deleted.
    */
    void DeleteSoundPtr(AudioStreamPtr sound);
	/*!
    \brief Reset a stream.
    \param sound The stream to be reseted.
    */
	void ResetSoundPtr(AudioStreamPtr sound);
    
    /*@}*/
    
    /**
     * @defgroup EffectsFunctions Effects creation and manipulation functions
     * @{
     */

    // Effect management
    /*!
	\brief Create an effect list.
	\return A pointer to new effect list.
	*/
    AudioEffectListPtr MakeAudioEffectListPtr();
    /*!
	\brief Add an effect in an effect list.
    \param list The effect list where the effect is added.
    \param effect The effect to be added.
	\return A pointer to the modified effect list.
	*/
    AudioEffectListPtr AddAudioEffectPtr(AudioEffectListPtr list_effect, AudioEffectPtr effect);
	/*!
	\brief Remove an effect from an effect list.
    \param list The effect list where the effect is removed.
    \param effect The effect to be removed.
	\return A pointer to the modified effect list.
	*/
    AudioEffectListPtr RemoveAudioEffectPtr(AudioEffectListPtr list_effect, AudioEffectPtr effect);
	/*!
	\brief Clear all effects from the list.
	\param list The effect list to be cleared.
	\return A pointer to the modified effect list.
	*/
	AudioEffectListPtr ClearAudioEffectListPtr(AudioEffectListPtr list_effect);
	/*!
	\brief Create a volume effect.
	\param gain The volume between 0 and 1.
    \return A pointer to new volume object.
	*/
    AudioEffectPtr MakeVolAudioEffectPtr(float vol);
	/*!
	\brief Create a mono pan effect.
	\param pan The pan between 0 and 1.
	\return A pointer to new mono pan object.
	*/
	AudioEffectPtr MakeMonoPanAudioEffectPtr(float pan);
	/*!
	\brief Create a stereo pan effect.
	\param panLeft The left pan between 0 and 1.
	\param rightPan The right pan between 0 and 1.
	\return A pointer to new stereo pan object.
	*/
	AudioEffectPtr MakeStereoPanAudioEffectPtr(float panLeft, float rightPan);
	/*!
	\brief Create a pitch shift effect.
	\param gain The pitch shift between 0.5 and 2.
    \return A pointer to new pitch shift object.
	*/
    AudioEffectPtr MakePitchShiftAudioEffectPtr(float pitch);
	/*!
	\brief Create an effect described in the Faust DSP language.
	\param name The name of the Faust effect shared library.
    \return A pointer to new effect object or NULL if the effect cannot be located or created.
	*/
	AudioEffectPtr MakeFaustAudioEffectPtr(const char* name);
	/*!
	\brief Create an effect by "wrapping" an externally built effect.
	\param effect The effect to be wrapped.
    \return A pointer to new effect object or NULL if the effect cannot be located or created.
	*/
	AudioEffectPtr MakeWrapperAudioEffectPtr(AudioEffectInterfacePtr effect);
	/*!
    \brief Return the number of effect controls.
    \param effect The effect pointer.
    \return The number of effect controls.
    */
	long GetControlCountEffectPtr(AudioEffectPtr effect);
	/*!
    \brief Return a description on the effect control: control name, min, max and default values
    \param effect The effect pointer.
    */
	void GetControlParamEffectPtr(AudioEffectPtr effect, long control, char* label, float* min, float* max, float* init);
	/*!
    \brief Set the effect control value.
    \param effect The effect pointer.
	\param control The control number between 0 and GetControlCountPtr.
	\param value The new value as a float.
    */
	void SetControlValueEffectPtr(AudioEffectPtr effect, long control, float value);	
	/*!
    \brief Get the effect control current value.
    \param effect The effect pointer.
	\param control The control number between 0 and GetControlCountPtr.
	\return The effect control current value.
	*/
	float GetControlValueEffectPtr(AudioEffectPtr effect, long control);	
	/*!
    \brief Set the effect running state.
	\param effect The effect to be used.
	\param state The running state.
	*/
	void SetStateEffectPtr(AudioEffectPtr effect, long state);	
	/*!
    \brief Get the effect running state.
	\param effect The effect to be used.
	\return state The running state.
	*/
	long GetStateEffectPtr(AudioEffectPtr effect);
	/*!
    \brief Reset the effect to intial state.
	\param effect The effect to be resetted.
	*/
	void ResetEffectPtr(AudioEffectPtr effect);
	/*!
    \brief Process an audio buffer with the effect.
	\param effect The effect to be used.
	\param input The input audio buffer.
	\param output The output audio buffer.
	\param framesNum The number of frame of input/output buffers.
	\param channels The number of channels of input/output buffers.
	*/
	void ProcessEffectPtr(AudioEffectPtr effect, float** input, float** output, long framesNum, long channels);
	/*!
    \brief Delete the effect list.
	\param list_effect The effect list pointer.
	*/
	void DeleteEffectListPtr(AudioEffectListPtr list_effect);	
	/*!
    \brief Delete the effect.
	\param effect The effect pointer.
	*/
	void DeleteEffectPtr(AudioEffectPtr effect);
    
    /*@}*/
    
    /**
     * @defgroup PlayerFunctions Player creation and manipulation functions
     * @{
     */

    // Open/Close
	/*!
    \brief Set global input/output audio latencies. This calls has to be done <B>before </B> OpenAudioPlayer 
	 and will take effect only when PortAudio is used.
    \param inputLatency The wanted input latency in millisecond.
    \param outputLatency The wanted output latency in millisecond.
    */
	void SetAudioLatencies(long inputLatency, long outputLatency);

    /*!
    \brief Open the audio player.
    \param inChan The number of input channels. <B>Only stereo players are currently supported </B>
    \param outChan The number of output channels.
    \param channels The number of stream channels.
    \param sample_rate The sampling rate.
    \param buffer_size The audio player internal buffer size.
    \param stream_buffer_size The file reader/writer buffer size (used for double buffering).
    \param rtstream_buffer_size The input stream buffer size.
    \param renderer The audio renderer used to access audio I/O : can be kPortAudioRenderer, kJackRenderer or kCoreAudioRenderer.
    \param thread_num The number of additionnal low-priority threads used to precompute data : must be a least one.
    \return A pointer to new audio player object.
    */
    AudioPlayerPtr OpenAudioPlayer(long inChan,
                                   long outChan,
                                   long channels,
                                   long sample_rate,
                                   long buffer_size,
                                   long stream_buffer_size,
                                   long rtstream_buffer_size,
                                   long renderer,
                                   long thread_num);
								   
	/*!
    \brief Close the audio player.
    \param player The audio player to be closed.
    */
    void CloseAudioPlayer(AudioPlayerPtr player);	
    
    // Transport
    /*!
    \brief Start the audio player.
    \param player The audio player.
    */
    void StartAudioPlayer(AudioPlayerPtr player);
    /*!
    \brief Stop the audio player.
    \param player The audio player.
    */
    void StopAudioPlayer(AudioPlayerPtr player);
  
    // Master
    /*!
    \brief Set the audio player volume [0...1]
    \param player The audio player.
    \param vol The new volume value.
    */
    void SetVolAudioPlayer(AudioPlayerPtr player, float vol);
    /*!
    \brief Set the audio player panning [0...1]
    \param player The audio player.
    \param pan The new panning value.
    */
    void SetPanAudioPlayer(AudioPlayerPtr player, float panLeft, float panRight);
	/*!
    \brief Set the master audio effect list.
    \param player The audio player.
    \param effect_list A list of audio effects.
	\param fadeIn The fadein length in frames.
    \param fadeOut The fadeout length in frames.
	*/
	void SetEffectListAudioPlayerPtr(AudioPlayerPtr player, AudioEffectListPtr effect_list, long fadeIn, long fadeOut);
    
    /*!
    \brief Get the audio player internal renderer.
    \param player The audio player.
    \return The internal audio renderer.
    */
    AudioRendererPtr GetAudioPlayerRenderer(AudioPlayerPtr player);
        
    /*@}*/
    
    /**
     * @defgroup ChannelFunctions Sound channel creation and manipulation functions
     * @{
     */
	
    /*!
    \brief Load a sound in a channel.
    \param player The audio player.
    \param sound The stream to be inserted in the channel.
    \param chan The audio channel number to be used.
    \param vol The volume between 0 and 1.
    \param pan The panning between 0 and 1.
    \return An error code.
    */
    long LoadChannelPtr(AudioPlayerPtr player, AudioStreamPtr sound, long chan, float vol, float panLeft, float panRight);
    /*!
    \brief Retrieve information about a sound channel.
    \param player The audio player.
    \param chan The audio channel number to be used.
    \param info The channel info structure to be filled.
    */
    void GetInfoChannel(AudioPlayerPtr player, long chan, ChannelInfoPtr info); // Obsolete version
    void GetChannelInfo(AudioPlayerPtr player, long chan, ChannelInfoPtr info);
	/*!
    \brief Set a callback to be called when the channel stops.
    \param player The audio player.
	\param chan The audio channel number to be used.
	\param callback The callback to be used.
	\param context A pointer to data to be given to the callback.
    */
	void SetStopCallbackChannel(AudioPlayerPtr player, long chan, StopCallback callback, void* context);
    /*!
    \brief Start a sound channel from the beginning.
    \param player The audio player.
    \param chan The audio channel number to be used.
    */	
    void StartChannel(AudioPlayerPtr player, long chan);
    /*!
    \brief Play a sound channel from the current location.
    \param player The audio player.
    \param chan The audio channel number to be used.
    */
    void ContChannel(AudioPlayerPtr player, long chan);
    /*!
    \brief Stop playing a channel, waiting for the stop fadeout to finish.
    \param player The audio player.
    \param chan The audio channel number to be used.
    */
    void StopChannel(AudioPlayerPtr player, long chan);
	/*!
    \brief Stop playing a channel, immediately returning without waiting for the stop fadeout to finish.
    \param player The audio player.
    \param chan The audio channel number to be used.
    */
	void AbortChannel(AudioPlayerPtr player, long chan);
    
    // Params
    /*!
    \brief Set the channel volume [0...1]
    \param player The audio player.
    \param chan The audio channel number to be used.
    \param vol The new volume value.
    */
    void SetVolChannel(AudioPlayerPtr player, long chan, float vol);
    /*!
    \brief Set the channel panning [0...1]
    \param player The audio player.
    \param chan The audio channel number to be used.
    \param pan The new panning value.
    */
    void SetPanChannel(AudioPlayerPtr player, long chan, float panLeft, float panRight);
	/*!
    \brief Set the channel audio effect list.
    \param player The audio player.
    \param chan The audio channel number to be used.
    \param effect_list A list of audio effects.
	\param fadeIn The fadein length in frames.
    \param fadeOut The fadeout length in frames.
	*/
	void SetEffectListChannelPtr(AudioPlayerPtr player, long chan, AudioEffectListPtr effect_list, long fadeIn, long fadeOut);
    
    /*@}*/
    
     /**
     * @defgroup RendererFunctions Low-level renderer and client creation and manipulation fonctions
     * @{
     */
    								   
	/*!
    \brief Opens the audio client to be added to an externally allocated renderer.
	\param renderer The audio renderer that will "drive" (call Audio callback) the player.
	\return A pointer to new audio player object.
	*/					
	AudioPlayerPtr OpenAudioClient(AudioRendererPtr renderer);	
	 							   						   
 	/*!
    \brief Close an audio client that was previously added to an externally allocated audio renderer using OpenAudioClient.
    \param player The audio client to be closed and be "detached" from the renderer.
    */					
	void CloseAudioClient(AudioPlayerPtr player);

	// Devices scanning
	/*!
	\brief Scan and return the number of available devices on the machine.
	\param renderer The audio renderer used to access audio I/O, built using MakeAudioRenderer.
	\return The number of available devices.
	*/
	long GetDeviceCount(AudioRendererPtr renderer);
	/*!
	\brief Fill DeviceInfo structure for a given device.
	\param renderer The audio renderer used to access audio I/O, built using MakeAudioRenderer.
	\param deviceNum The device index between 0 and GetDeviceCount.	
	\param info The device info structure to be filled.
	*/
	void GetDeviceInfo(AudioRendererPtr renderer, long deviceNum, DeviceInfo* info);
	/*!
	\brief Get the default input device index.
	\param renderer The audio renderer used to access audio I/O, built using MakeAudioRenderer.
	\return The default input device index.
	*/
	long GetDefaultInputDevice(AudioRendererPtr renderer);
	/*!
	\brief Get the default output device index.
	\param renderer The audio renderer used to access audio I/O, built using MakeAudioRenderer.
	\return The default output device index.
	*/
	long GetDefaultOutputDevice(AudioRendererPtr renderer);
	
	// Renderer
	/*!
    \brief Create a new audio renderer.
    \param renderer The audio renderer used to access audio I/O : can be kPortAudioRenderer, kJackRenderer or kCoreAudioRenderer.
	\return A pointer to new audio renderer object.
	*/
	AudioRendererPtr MakeAudioRenderer(long renderer);
	/*!
    \brief Delete an audio renderer.
    \param renderer The renderer to be deleted.
	*/
	void DeleteAudioRenderer(AudioRendererPtr renderer);
	/*!
    \brief Open the audio renderer.
    \param renderer The audio renderer used.
    \param inputDevice The audio input device index.
    \param outputDevice The audio output device index.
	\param inChan The number of input channels. <B>Only stereo players are currently supported </b>. On input, contains the wanted value, on return the really used one.
	\param outChan The number of output channels.  On input, contains the wanted value, on return the really used one.
	\param bufferSize The audio player internal buffer size.  On input, contains the wanted value, on return the really used one.
    \param sampleRate The sampling rate. On input, contains the wanted value, on return the really used one.
 	\return An error code.
	*/
	int OpenAudioRenderer(AudioRendererPtr renderer, long inputDevice, long outputDevice, long inChan, long outChan, long bufferSize, long sampleRate);  
	/*!
    \brief Close an audio renderer.
    \param renderer The audio renderer to be closed.
	*/
	void CloseAudioRenderer(AudioRendererPtr renderer); 
	
	/*!
    \brief Start an audio renderer.
    \param renderer The audio renderer to be started.
	*/	
	void StartAudioRenderer(AudioRendererPtr renderer); 
	/*!
    \brief Stop an audio renderer.
    \param renderer The audio renderer to be stopped.
	*/
	void StopAudioRenderer(AudioRendererPtr renderer); 
    
    /*!
    \brief Get audio renderer infos.
    \param renderer The audio renderer.
    \param info The audio renderer info to be filled.
	*/
	void GetAudioRendererInfo(AudioRendererPtr renderer, RendererInfoPtr info); 
	
	/*!
    \brief Add an audio client to the renderer internal client list.
    \param renderer The audio renderer to be used.
	\param client The audio client to be added.
	*/
	void AddAudioClient(AudioRendererPtr renderer, AudioClientPtr client); 
	/*!
    \brief Remove an audio client from the renderer internal client list.
    \param renderer The audio renderer to be used.
	\param client The audio client to be removed.
	*/
	void RemoveAudioClient(AudioRendererPtr renderer, AudioClientPtr client); 
	
	/*!
    \brief Init the global audio context. There is <B> unique </B> to be accessed by all components that need it.
    \param inChan The number of input channels. <B>Only stereo players are currently supported </b>
    \param outChan The number of output channels.
    \param channels The number of stream channels.
    \param sample_rate The sampling rate.
    \param buffer_size The audio player internal buffer size.
    \param stream_buffer_size The file reader/writer buffer size (used for double buffering).
    \param rtstream_buffer_size The input stream buffer size.
    \param thread_num The number of additionnal low-priority threads used to precompute data : must be a least one.
    */
	void AudioGlobalsInit(long inChan, 
						long outChan, 
						long channels, 
						long sample_rate,
						long buffer_size, 
						long stream_buffer_size, 
						long rtstream_buffer_size,
						long thread_num);
	/*!
    \brief Destroy the global audio context.
  	*/
	void AudioGlobalsDestroy();
    
    /*@}*/

/*! @} */

#ifdef __cplusplus
}
#endif

#endif


