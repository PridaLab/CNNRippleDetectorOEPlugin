#include "RecordEnginePlugin.h"

RecordEnginePlugin::RecordEnginePlugin();
	
RecordEnginePlugin::~RecordEnginePlugin();

String RecordEnginePlugin::getEngineID() const 
{
	return "RecordEngine Name";
}

void RecordEnginePlugin::setParameter(EngineParameter& parameter)
{

}

void RecordEnginePlugin::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
	// called when files should be opened
}

void RecordEnginePlugin::closeFiles()
{
	// called when files should be closed
}

void RecordEnginePlugin::startChannelBlock(bool lastBlock)
{

}

void RecordEnginePlugin:: writeData(int writeChannel, int realChannel, const float* buffer, int size)
{

}

void RecordEnginePlugin::writeSynchronizedData(int writeChannel, 
											   int realChannel, 
											   const float* dataBuffer, 
											   const double* ftsBuffer, 
											   int size)
{

}

void RecordEnginePlugin::endChannelBlock(bool lastBlock)
{

}

void RecordEnginePlugin::writeEvent(int eventChannel, const MidiMessage& event)
{

}

void RecordEnginePlugin::writeTimestampSyncText(uint16 sourceID, uint16 sourceIdx, int64 timestamp, float sourceSampleRate, String text)
{

}

void RecordEnginePlugin::registerProcessor(const GenericProcessor* processor)
{

}

void RecordEnginePlugin::addDataChannel(int index, const DataChannel* chan)
{

}

void RecordEnginePlugin::addEventChannel(int index, const EventChannel* chan)
{

}

void RecordEnginePlugin::registerSpikeSource(const GenericProcessor* processor)
{

}

void RecordEnginePlugin::addSpikeElectrode(int index, const SpikeChannel* elec)
{

}

void RecordEnginePlugin::writeSpike(int electrodeIndex, const SpikeEvent* spike)
{

}

void RecordEnginePlugin::resetChannels()
{

}

void RecordEnginePlugin::updateTimestamps(const Array<int64>& timestamps, int channel = -1)
{

}

void RecordEnginePlugin::setChannelMapping (const Array<int>& channels, 
										    const Array<int>& chanProcessor, 
											const Array<int>& chanOrder, 
											OwnedArray<RecordProcessorInfo>& processors)
{

}

void RecordEnginePlugin::registerRecordNode(RecordNode* node)
{

}

void RecordEnginePlugin::startAcquisition()
{

}

void RecordEnginePlugin::directoryChanged()
{

}