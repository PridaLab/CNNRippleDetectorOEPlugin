//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef FILESOURCEPLUGIN_H_DEFINED
#define FILESOURCEPLUGIN_H_DEFINED

#include <ProcessorHeaders.h>

class FileSourcePlugin : public FileSource
{
public:
	/** The class constructor, used to initialize any members. */
	FileSourcePlugin();

	/** The class destructor, used to deallocate memory */
	~FileSourcePlugin();

	int readData (int16* buffer, int nSamples);

    void processChannelData (int16* inBuffer, float* outBuffer, int channel, int64 numSamples);

    void seekTo (int64 sample);

	bool Open (File file);
    
	void fillRecordInfo();

    void updateActiveRecord();

};

#endif