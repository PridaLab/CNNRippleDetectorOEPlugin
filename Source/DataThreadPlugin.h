//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef DATATHREADPLUGIN_H_DEFINED
#define DATATHREADPLUGIN_H_DEFINED

#include <ProcessorHeaders.h>

class DataThreadPlugin : public DataThread
{
public:
	/** The class constructor, used to initialize any members. */
	DataThreadPlugin();

	/** The class destructor, used to deallocate memory */
	~DataThreadPlugin();

	/** Called repeatedly to add any available data to the buffer */
	bool updateBuffer();

    /** Returns true if the data source is connected, false otherwise.*/
    bool foundInputSource();

    /** Initializes data transfer.*/
    bool startAcquisition();

    /** Stops data transfer.*/
    bool stopAcquisition();

    /** Returns the number of continuous headstage channels the data source can provide.*/
    int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx);

	/** Returns the number of TTL channels that each subprocessor generates*/
	int getNumTTLOutputs(int subProcessorIdx);

    /** Returns the sample rates of the data source, by subprocessor.*/
    float getSampleRate(int subProcessorIdx);

	/** Returns the number of virtual subprocessors this source can generate */
	unsigned int getNumSubProcessors();

	/** Called to create extra event channels, apart from the default TTL ones*/
	void createExtraEvents(Array<EventChannel*>& events);

    /** Returns the volts per bit of the data source.*/
    float getBitVolts (const DataChannel* chan);

	/** Returns true if the data source has custom channel names */
	bool usesCustomNames();

	/** Sets the custom names for each channel */
	void setDefaultChannelNames();

};

#endif