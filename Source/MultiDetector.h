//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef MULTIDETECTOR_H_DEFINED
#define MULTIDETECTOR_H_DEFINED

#include <ProcessorHeaders.h>
#include "tf_functions.hpp"

#define MAX_ROUND_BUFFER_SIZE 3000
#define NUM_CHANNELS 8

//namespace must be an unique name for your plugin
namespace MultiDetectorSpace
{
	class MultiDetector : public GenericProcessor
	{
	public:
		/** The class constructor, used to initialize any members. */
		MultiDetector();

		/** The class destructor, used to deallocate memory */
		~MultiDetector();

		/** Indicates if the processor has a custom editor. Defaults to false */
		bool hasEditor() const { return true; }

		/** If the processor has a custom editor, this method must be defined to instantiate it. */
		AudioProcessorEditor* createEditor() override;

		/** Optional method that informs the GUI if the processor is ready to function. If false acquisition cannot start. Defaults to true */
		//bool isReady();

		/** Defines the functionality of the processor.

		The process method is called every time a new data buffer is available.

		Processors can either use this method to add new data, manipulate existing
		data, or send data to an external target (such as a display or other hardware).

		Continuous signals arrive in the "buffer" variable, event data (such as TTLs
		and spikes) is contained in the "events" variable.
		*/
		void process(AudioSampleBuffer& buffer) override;

		/** Handles events received by the processor

		Called automatically for each received event whenever checkForEvents() is called from process()		
		*/
		//void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition) override;

		/** Handles spikes received by the processor

		Called automatically for each received event whenever checkForEvents(true) is called from process()
		*/
		//void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

		/** The method that standard controls on the editor will call.
		It is recommended that any variables used by the "process" function
		are modified only through this method while data acquisition is active. */
		//void setParameter(int parameterIndex, float newValue) override;

		/** Saving custom settings to XML. */
		//void saveCustomParametersToXml(XmlElement* parentElement) override;

		/** Load custom settings from XML*/
		//void loadCustomParametersFromXml() override;

		/** Optional method called every time the signal chain is refreshed or changed in any way.

		Allows the processor to handle variations in the channel configuration or any other parameter
		passed down the signal chain. The processor can also modify here the settings structure, which contains
		information regarding the input and output channels as well as other signal related parameters. Said
		structure shouldn't be manipulated outside of this method.

		*/
		//void updateSettings() override;


		bool enable() override;
		bool disable() override;

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
		void setTimeout(int newTimeout);
		void setPulseDuration(int newPulseDuration);
		void setCalibrationTime(float newCalibrationTime);
		void setThreshold1(float newThreshold);
		void setThreshold2(float newThreshold);
		void setInputLayer(const String& newInputLayer);
		void setChannel1(int channel);
		void setChannel2(int channel);

	private:

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
		unsigned int predictBufferSize;
		int effectiveStride;

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
}

#endif