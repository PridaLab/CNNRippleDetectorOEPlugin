//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef RECORDENGINEPLUGIN_H_DEFINED
#define RECORDENGINEPLUGIN_H_DEFINED

#include <ProcessorHeaders.h>

class RecordEnginePlugin : public RecordEngine
{
public:
	RecordEnginePlugin();
	
	~RecordEnginePlugin();

	String getEngineID() const;

	/** All the public methods (except registerManager) are called by RecordNode or RecordingThread:

	When acquisition starts (in the specified order):
	1-resetChannels
	2-registerProcessor, addChannel, registerSpikeSource, addSpikeElectrode
	3-configureEngine (which calls setParameter)
	3-startAcquisition

	When recording starts (in the specified order):
	1-directoryChanged (if needed)
	2-(setChannelMapping)
	3-(updateTimestamps*)
	4-openFiles*

	During recording: (RecordThread loop)
	1-(updateTimestamps*) (can be called in a per-channel basis when the circular buffer wraps)
	2-startChannelBlock*
	3-writeData* (per channel. Can be called more than once to account for the circular buffer wrap)
	4-endChannelBlock*
	4-writeEvent* (if needed)
	5-writeSpike* (if needed)

	When recording stops:
	closeFiles*
	Methods marked with a * are called via the RecordThread thread.
	Methods marked with parenthesis are not overloaded methods
	*/

	/** Called for registering parameters */
	void setParameter(EngineParameter& parameter);

	/** Called when recording starts to open all needed files */
	void openFiles(File rootFolder, int experimentNumber, int recordingNumber);

	/** Called when recording stops to close all files and do all the necessary cleanups */
	void closeFiles();

	/** Called by the record thread before it starts writing the channels to disk */
	void startChannelBlock(bool lastBlock);

	/** Write continuous data for a channel. The raw buffer pointer is passed for speed, care must be taken to only read the specified number of bytes. */
	void writeData(int writeChannel, int realChannel, const float* buffer, int size);

	/** Write continuous data for a channel with synchronized float timestamps */
	void writeSynchronizedData(int writeChannel, int realChannel, const float* dataBuffer, const double* ftsBuffer, int size);

	/** Called by the record thread after it has written a channel block */
	void endChannelBlock(bool lastBlock);

	/** Write a single event to disk.  */
	void writeEvent(int eventChannel, const MidiMessage& event);

	/** Handle the timestamp sync text messages*/
	void writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float sourceSampleRate, String text);

	/** Called when acquisition starts once for each processor that might record continuous data */
	void registerProcessor(const GenericProcessor* processor);

	/** Called after registerProcessor, once for each output channel of the processor */
	void addDataChannel(int index, const DataChannel* chan);

	/** Called after registerProcessor, once for each output channel of the processor */
	void addEventChannel(int index, const EventChannel* chan);

	/** Called when acquisition starts once for each processor that might record spikes */
	void registerSpikeSource(const GenericProcessor* processor);

	/** Called after registerSpikesource, once for each channel group */
	void addSpikeElectrode(int index, const SpikeChannel* elec);

	/** Write a spike to disk */
	void writeSpike(int electrodeIndex, const SpikeEvent* spike);

	/** Called when a new acquisition starts, to clean all channel data before registering the processors */
	void resetChannels();

	/** Called at the start of every write block */
	void updateTimestamps(const Array<int64>& timestamps, int channel = -1);

	/** Called prior to opening files, to set the map between recorded channels and actual channel numbers */
	void setChannelMapping (const Array<int>& channels, const Array<int>& chanProcessor, const Array<int>& chanOrder, OwnedArray<RecordProcessorInfo>& processors);

	void registerRecordNode(RecordNode* node);

	/** Called after all channels and spike groups have been registered, just before acquisition starts */
	void startAcquisition();

	/** Called when the recording directory changes during an acquisition */
	void directoryChanged();

};

#endif