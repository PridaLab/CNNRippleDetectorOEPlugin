//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef MULTIDETECTOR_H_DEFINED
#define MULTIDETECTOR_H_DEFINED

#include <ProcessorHeaders.h>
#include "tf_functions.hpp"

#define MAX_ROUND_BUFFER_SIZE 3000
#define NUM_CHANNELS 8

class MultiDetectorEditor;

class MultiDetectorSettings
{
	public:

		/** Constructor -- sets default values **/
		MultiDetectorSettings();

		/** Destructor **/
		~MultiDetectorSettings() {}

		/** Creates an event associated with Ripple Detection */
		TTLEventPtr createEvent(int64 outputLine, int64 sample_number, bool state);

		Array<int> inputChannels;

		int downsampleFactor;

		int pulseDuration;
		int pulseDurationSamples;

		/* Calibration */
		int calibrationTime;
		std::vector<std::vector<float>> calibrationBuffer;
		std::vector<double> channelMeans;
		int elapsedCalibrationPoints;
		bool isCalibrated;

		int drift;

		int timeout;
		int timeoutSamples;
		int timeoutDownsampled;

		int threshold;
		int thresholdSign;

		int outputChannel;

		/** TTL event channel */
		EventChannel* eventChannel;

};

class MultiDetector : public GenericProcessor
{
public:
	/** The class constructor, used to initialize any members. */
	MultiDetector();

	/** The class destructor, used to deallocate memory */
	~MultiDetector();

	/** Creates the custom editor for this plugin */
	AudioProcessorEditor* createEditor() override;

	/** Processes data coming into the plugin */
	void process(AudioSampleBuffer& buffer) override;

	/** Called when a processor needs to update its settings */
	void updateSettings() override;

	/** Called when a parameter is updated */
    void parameterValueChanged(Parameter* param) override;

	bool enable();
	bool disable();

	bool setFile(String fullpath);

	float getPredictBufferSize();
	void setPredictBufferSize(float newPredictBufferSize);
	float getStride();
	void setStride(float newStride);

	int getTimeout();
	int getPulseDuration();
	float getCalibrationTime();
	float getThreshold1();
	float getThreshold2();
	String getInputLayer();
	float getThrDrift();

	void setTimeout(int newTimeout);
	void setPulseDuration(int newPulseDuration);
	void setCalibrationTime(float newCalibrationTime);
	void setThreshold1(float newThreshold);
	void setThreshold2(float newThreshold);
	void setInputLayer(const String& newInputLayer);
	void setChannel1(int channel);
	void setChannel2(int channel);
	void setThrDrift(float newThrDrift);

private:

	MultiDetectorEditor* ed;

	StreamSettings<MultiDetectorSettings> settings;

	float calculateMean(std::vector<float> data);
	float calculateStd(std::vector<float> data, float mean);
	void pushMeanStd(double x, int chan);
	double getMean(int chan);
	double getStd(int chan);

	void createEventChannels();
	void sendTTLEvent1(uint64 bufferTs, int bufferNumSamples, int sample_index, int eventChannel);
	void sendTTLEvent2(uint64 bufferTs, int bufferNumSamples, int sample_index, int eventChannel);

	EventChannel *ttlEventChannel;

	String modelPath;
	bool modelLoaded;

	bool isCalibration;
	float calibrationTime;
	int elapsedCalibration;
	std::vector<double> channelsOldStds, channelsNewStds, channelsStds;
	std::vector<double> channelsOldMeans, channelsNewMeans, channelsMeans;
	std::vector<std::vector<float>> calibrationBuffer;

	float roundBuffer[MAX_ROUND_BUFFER_SIZE][NUM_CHANNELS];
	unsigned int roundBufferWriteIndex;
	unsigned int roundBufferReadIndex;
	unsigned int roundBufferNumElements;

	std::vector<float> predictBuffer;
	std::vector<float> predictBufferSum;
	unsigned int predictBufferSize;
	int effectiveStride;
	float thrDrift;
	bool skipPrediction;

	float samplingRate;
	float downsampledSamplingRate;
	unsigned int downsampleFactor;
	unsigned int loopIndex;
	unsigned int sinceLast;

	int timeout;
	int pulseDuration;
	int timeoutSamples;
	int pulseDurationSamples;
	int timeoutDownsampled;
	String inputLayer;

	int nextSampleEnable;
	int globalSample;
	unsigned int forwardSamples;

	float threshold1;
	float threshold2;
	float thresholdSign1;
	float thresholdSign2;

	int channel1;
	int channel2;

	TTLEventPtr turnoffEvent1; // Variable to store a turn off event that should go in the next buffer
	TTLEventPtr turnoffEvent2;

	TF_Graph * graph = nullptr;
	TF_Session * session = nullptr;
	TF_Output input, output;

};

#endif