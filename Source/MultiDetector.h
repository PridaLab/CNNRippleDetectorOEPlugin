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
		int elapsedCalibrationPoints;
		bool isCalibrating;

		int drift;

		int timeout;
		int timeoutSamples;
		int timeoutDownsampled;

		float threshold;
		int thresholdSign;
    
        /* Sample buffering */
        int globalSample;
        int forwardSamples;
        int nextSampleEnable;
    
        std::vector<double> channelsOldStds, channelsNewStds, channelsStds;
        std::vector<double> channelsOldMeans, channelsNewMeans, channelsMeans;

        float roundBuffer[MAX_ROUND_BUFFER_SIZE][NUM_CHANNELS];
		int roundBufferWriteIndex;
		int roundBufferReadIndex;
		int roundBufferNumElements;
        unsigned int sinceLast;

		std::vector<float> predictBuffer;
		std::vector<float> predictBufferSum;
    
        bool skipPrediction;

		int outputChannel;
    
        TTLEventPtr turnoffEvent1;
        TTLEventPtr turnoffEvent2;

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
	void process(AudioBuffer<float>& buffer) override;

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
	void pushMeanStd(uint64 streamId, double x, int chan);
	double getMean(uint64 streamId, int chan);
	double getStd(uint64 streamId, int chan);
    
    void predict(uint64 streamId);

	void createEventChannels();
	void sendTTLEvent(uint64 streamId, uint64 bufferTs, int bufferNumSamples, int sample_index);

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
	unsigned int predictBufferSize {16};
	int effectiveStride {8};
	float thrDrift;
	bool skipPrediction;

	float samplingRate;
	int downsampledSamplingRate {1250};
	unsigned int downsampleFactor;
	unsigned int loopIndex;

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
