#include "MultiDetector.h"
#include "MultiDetectorEditor.h"
#include <cmath>


#define MAX_PREDICT_BUFFER_SIZE 16
#define EFFECTIVE_STRIDE 8 // Lower stride, faster detection, more computation cost
#define SIGMOID_THRESH 0.84729786 //0.40565
#define LOGIT(p) log(p/(1.-p))
#define SIGMOID(x) 1./(1.+exp(-x))

MultiDetectorSettings::MultiDetectorSettings()
{
	eventChannel = nullptr;
}

TTLEventPtr MultiDetectorSettings::createEvent(int64 outputLine, int64 sample_number, bool state)
{
	TTLEventPtr event = TTLEvent::createTTLEvent(
		eventChannel,
		sample_number,
		outputLine, 
		state
	);

	return event;
}


MultiDetector::MultiDetector() : GenericProcessor("CNN-ripple")
{

	addFloatParameter(
		Parameter::STREAM_SCOPE,
		"pulse_duration",
		"Pulse duration (in ms)",
		 48, 	//default 
		 0, 	//min
		 9999,  //max
		 1  	//step
	);

	addFloatParameter(
		Parameter::STREAM_SCOPE,
		"timeout",
		"Minimum time between events (in ms)",
		 48, 	//default 
		 0, 	//min
		 9999,  //max
		 1  	//step
	);

	addFloatParameter(
		Parameter::STREAM_SCOPE,
		"calibration_time",
		"Duration of calibration time (in s)",
		 60, 	//default 
		 0, 	//min
		 9999,  //max
		 1  	//step
	);

	addFloatParameter(
		Parameter::STREAM_SCOPE,
		"threshold",
		"Probability threshold",
		 0, 	//default 
		 0, 	//min
		 1,     //max
		 0.01  	//step
	);

	addFloatParameter(
		Parameter::STREAM_SCOPE,
		"drift",
		"Drift prevention threshold (standard deviations)",
		 0, 	//default 
		 0, 	//min
		 20,    //max
		 0.1  	//step
	);

	addIntParameter(
		Parameter::STREAM_SCOPE,
		"output",
		"The output TTL line",
		1, 		//deafult
		1, 		//min
		16		//max
	);

	//setProcessorType(PROCESSOR_TYPE_FILTER);

	/*
	predictBufferSize = 16;
	effectiveStride = 8;

	downsampledSamplingRate = 1250.0;
	downsampleFactor = 1.0;
	loopIndex = 0;
	sinceLast = effectiveStride;

	roundBufferWriteIndex = 0;
	roundBufferReadIndex = 0;
	roundBufferNumElements = 0;

	modelPath = "";
	modelLoaded = false;

	pulseDuration = 48.0;
	timeout = 48.0;
	nextSampleEnable = 0;
	globalSample = 0;
	forwardSamples = 0;

	threshold1 = 0.5;
	thresholdSign1 = 1;
	threshold2 = 0.5;
	thresholdSign2 = 1;
	inputLayer = "conv1d_input";
	turnoffEvent1 = nullptr;
	turnoffEvent2 = nullptr;

	channel1 = -1;
	channel2 = -1;

	calibrationBuffer = std::vector<std::vector<float>>(NUM_CHANNELS);
	calibrationTime = 60 * 1; // sec
	elapsedCalibration = 0; // points
	isCalibration = true;
	channelsStds = std::vector<double>(NUM_CHANNELS);
	channelsMeans = std::vector<double>(NUM_CHANNELS);
	channelsNewStds = std::vector<double>(NUM_CHANNELS);
	channelsNewMeans = std::vector<double>(NUM_CHANNELS);
	channelsOldStds = std::vector<double>(NUM_CHANNELS);
	channelsOldMeans = std::vector<double>(NUM_CHANNELS);

	thrDrift = 0.;

	for (int i = 0; i < NUM_CHANNELS; i++) {
		calibrationBuffer[i] = std::vector<float>(calibrationTime * downsampledSamplingRate);
		channelsMeans[i] = 0.;
	}

	printf("Sampling rate %f Downsample factor %d\n", samplingRate, downsampleFactor);
	printf("nInputs %d nOutputs %d\n", getNumInputs(), getNumOutputs());
	*/
}

MultiDetector::~MultiDetector()
{
	tf_functions::delete_graph(graph);

	//TODO: Causes crash on GUI close if no model is loaded in 
	//tf_functions::delete_session(session);
}

void MultiDetector::updateSettings()
{

	settings.update(getDataStreams());

	for (auto stream : getDataStreams())
	{

		parameterValueChanged(stream->getParameter("pulse_duration"));
		parameterValueChanged(stream->getParameter("timeout"));
		parameterValueChanged(stream->getParameter("calibration_time"));
		parameterValueChanged(stream->getParameter("threshold"));
		parameterValueChanged(stream->getParameter("drift"));
		parameterValueChanged(stream->getParameter("output"));

		//Add event channel
		EventChannel::Settings s {
			EventChannel::Type::TTL,
			"Ripple detector output",
			"Triggers when a ripple is detected on the input channel",
			"dataderived.ripple",
			getDataStream(stream->getStreamId())
		};
		eventChannels.add(new EventChannel(s));
		eventChannels.getLast()->addProcessor(processorInfo.get());
		settings[stream->getStreamId()]->eventChannel = eventChannels.getLast();
	}

}


AudioProcessorEditor* MultiDetector::createEditor()
{
	 editor = std::make_unique<MultiDetectorEditor>(this);

	return editor.get();
}

void MultiDetector::parameterValueChanged(Parameter* param)
{
	//TODO
}

bool MultiDetector::enable()
{
	/*
	const DataChannel* inChan = getDataChannel(0);
	if (inChan == nullptr) {
		printf("No input channels.\n");
		return false;
	}
	samplingRate = inChan->getSampleRate();

	downsampleFactor = (unsigned int)samplingRate / downsampledSamplingRate;
	pulseDurationSamples = int(std::ceil(pulseDuration * samplingRate / 1000.0f));
	timeoutSamples = int(std::floor(timeout * samplingRate / 1000.0f));
	timeoutDownsampled = int(std::floor(timeoutSamples / downsampleFactor));


	if (modelLoaded == false) {
		printf("Model not loaded yet.\n");
		return false;
	}


	// Restart round buffer
	roundBufferWriteIndex = 0;
	roundBufferReadIndex = 0;
	roundBufferNumElements = 0;


	predictBuffer = std::vector<float>(predictBufferSize * NUM_CHANNELS);
	predictBufferSum = std::vector<float>(predictBufferSize);
	*/

	return true;
}


bool MultiDetector::disable()
{
	// TODO

	return true;
}


void MultiDetector::process(AudioSampleBuffer& buffer)
{
	/**
	If the processor needs to handle events, this method must be called onyl once per process call
	If spike processing is also needing, set the argument to true
	*/

	/*
	//checkForEvents(false);
	int numChannels = buffer.getNumChannels();
	// TODO: check that numChannels >= 8

	// We use this instead of buffer.getNumSamples() because the second returns all the buffer positions,
	// even the empty ones. The first just gives the number of used positions.
	int numSamples = getNumSamples(0);

	uint64 tsBuffer = getTimestamp(0); // pts

	unsigned int ts = (unsigned int)(1000.f * float(tsBuffer) / samplingRate); // ms

	if (modelLoaded == false) {
		printf("Model not loaded yet.\n");
		return;
	}


	//std::cout << "samples: " << numSamples << " time: " << ts << " fs: " << samplingRate << " factor: " << downsampleFactor << " nextSampleEnable: " << nextSampleEnable << std::endl;


	// Gets pointers to buffers
	const float* channelsData[NUM_CHANNELS];
	for (int chan = 0; chan < NUM_CHANNELS; chan++) {
		channelsData[chan] = buffer.getReadPointer(chan);
	}


	// Sends TTL events that were stored from previous buffers if its time
	int turnoffOffset1 = turnoffEvent1 ? juce::jmax(0, int(turnoffEvent1->getTimestamp() - tsBuffer)) : -1;
	if (turnoffOffset1 >= 0 && turnoffOffset1 < numSamples) {
		addEvent(ttlEventChannel, turnoffEvent1, turnoffOffset1);
		turnoffEvent1 = nullptr;
	}

	int turnoffOffset2 = turnoffEvent2 ? juce::jmax(0, int(turnoffEvent2->getTimestamp() - tsBuffer)) : -1;
	if (turnoffOffset2 >= 0 && turnoffOffset2 < numSamples) {
		addEvent(ttlEventChannel, turnoffEvent2, turnoffOffset2);
		turnoffEvent2 = nullptr;
	}


	for (int sample = 0; sample < numSamples; sample++, globalSample++) {

		// Save sample in round buffer
		if (globalSample % downsampleFactor == 0) {
			// Use globalSample so it is not relative to the buffer
			globalSample = 0;

			if (isCalibration == true) {

				elapsedCalibration++;

				for (int chan = 0; chan < NUM_CHANNELS; chan++) {
					//calibrationBuffer[chan][elapsedCalibration-1] = channelsData[chan][sample];
					//channelsMeans[chan] += channelsData[chan][sample];
					pushMeanStd(channelsData[chan][sample], chan);
				}

				if (elapsedCalibration >= (calibrationTime * downsampledSamplingRate)) {
					isCalibration = false;
					for (int chan = 0; chan < NUM_CHANNELS; chan++) {
						//channelsMeans[chan] /= calibrationBuffer[chan].size();
						//channelsStds[chan] = calculateStd(calibrationBuffer[chan], channelsMeans[chan]);
						//if (channelsStds[chan] == 0) channelsStds[chan] = 1;
						channelsMeans[chan] = getMean(chan);
						channelsStds[chan] = getStd(chan);
					}
				}
			}

			for (int chan = 0; chan < NUM_CHANNELS; chan++) {
				roundBuffer[roundBufferWriteIndex][chan] = channelsData[chan][sample];
			}

			roundBufferWriteIndex = (roundBufferWriteIndex + 1) % MAX_ROUND_BUFFER_SIZE;
			if (roundBufferNumElements < predictBufferSize) roundBufferNumElements++;
			sinceLast++;
		}

		//std::cout << "sinceLast: " << sinceLast << " nextSampleEnable: " << nextSampleEnable << std::endl;

		// Check if it is time to predict (enough data AND effective stride passed AND timeout passed)
		if ((roundBufferNumElements >= predictBufferSize) && (sinceLast >= effectiveStride) && (sample >= nextSampleEnable)) {
			//std::cout << "sinceLast: " << sinceLast << " elapsedCalibration: " << elapsedCalibration << std::endl;
			sinceLast = 0;
			nextSampleEnable = sample + 1;

			//std::cout << "Sample: " << sample << " Downsample: " << sample/downsampleFactor << " Time: " << (unsigned int)(1000.f * float(tsBuffer + sample) / samplingRate) << std::endl;
			//std::cout << "Write: " << roundBufferWriteIndex << " Read: " << roundBufferReadIndex << std::endl;

			unsigned int temporalReadIndex = roundBufferReadIndex;
			unsigned int oldRoundBufferReadIndex = roundBufferReadIndex;
			// Next value to read will be the first upcoming window after stride, if no event is found
			roundBufferReadIndex = (oldRoundBufferReadIndex + effectiveStride) % MAX_ROUND_BUFFER_SIZE;

			// If still calibrating do nothing yet
			if (isCalibration == true) continue;

			// Create predict window
			for (int idx = 0; idx < predictBufferSize; idx++) {
				for (int chan = 0; chan < NUM_CHANNELS; chan++) {
					// Save in the buffer after z-score norm. It is done here because the mean and std are already calculated
					predictBuffer[(idx * NUM_CHANNELS) + chan] = (roundBuffer[temporalReadIndex][chan] - channelsMeans[chan]) / channelsStds[chan];
					predictBufferSum[idx] += predictBuffer[(idx * NUM_CHANNELS) + chan];
				}
				predictBufferSum[idx] = abs(predictBufferSum[idx]/NUM_CHANNELS);
				temporalReadIndex = (temporalReadIndex + 1) % MAX_ROUND_BUFFER_SIZE;
			}
			// If drift threshold is bigger than 0 then check the channels absolute mean
			if (thrDrift > 0) {
				skipPrediction = true;
				float meanWindow = 0;
				for (int idx = 0; idx < predictBufferSize; idx++) {
					meanWindow += predictBufferSum[idx];
					predictBufferSum[idx] = 0;
				}
				//for (int chan = 0; chan < NUM_CHANNELS; chan++) {
				//	float meanChan += predictBufferSum[chan] ;
				//	predictBufferSum[chan] = 0;

					// If just one channel is under the threshold then perform prediction
				//	if (meanChan < (channelsMeans[chan] + (thrDrift * channelsStds[chan]))) {
				//		skipPrediction = false;
				//		break;
				//	}
				//}
				meanWindow = meanWindow/(predictBufferSize);
				if (meanWindow < thrDrift) {
					skipPrediction = false;
				}
			}


			//Predict
			if (skipPrediction == false) {
				// FILE * f = fopen("salida4.txt", "a");

				// for (int idx = 0; idx < predictBufferSize; idx++) {
				// 	fprintf(f, "%f ", predictBuffer[(idx * NUM_CHANNELS) + 0]);
				// }
				// fprintf(f, "\n");
				// fclose(f);

				TF_Tensor* input_tensor = nullptr, * output_tensor = nullptr;


				std::vector<std::int64_t> dims = { 1, predictBufferSize, NUM_CHANNELS };
				int num_dims = 3;
				tf_functions::create_tensor(TF_FLOAT, dims, num_dims, predictBuffer, &input_tensor);

				tf_functions::run_session(session, &input, &input_tensor, 1, &output, &output_tensor, 1);

				// Check results
				auto tensor_data = static_cast<float*>(TF_TensorData(output_tensor));
				//std::cout << tensor_data[0] << std::endl;

				forwardSamples = 0;

				// Event 0
				if ((thresholdSign1 * tensor_data[0]) >= (thresholdSign1 * threshold1) && channel1 >= 0) {
					forwardSamples = 1;

					//std::cout << tsBuffer << " " << numSamples << " " << sample << " " << tensor_data[0] << std::endl;
					sendTTLEvent1(tsBuffer, numSamples, sample, channel1);
				}

				// Event 2
				if ((thresholdSign2 * tensor_data[2]) >= (thresholdSign2 * threshold2) && channel2 >= 0) {
					forwardSamples = 1;

					//std::cout << tensor_data[2] << " " << tensor_data[7] << std::endl;
					sendTTLEvent2(tsBuffer, numSamples, sample, channel2);
				}

				if (forwardSamples == 1) {
					nextSampleEnable += timeoutSamples - 1;
					//std::cout <<  "event" << " nextSample " << nextSampleEnable << std::endl;
					// If an event has been found, next value to read will be the first upcoming window after timeout
					roundBufferReadIndex = (oldRoundBufferReadIndex + std::max(timeoutDownsampled, effectiveStride)) % MAX_ROUND_BUFFER_SIZE;
				}

				tf_functions::delete_tensor(input_tensor);
				tf_functions::delete_tensor(output_tensor);
			}
		}
	}


	// Shift nextSampleEnable so it is relative to the next buffer
	nextSampleEnable = juce::jmax(0, nextSampleEnable - numSamples);

	*/
}



void MultiDetector::createEventChannels() {

	/*
	int num_of_ttl_channels = 8;
	ttlEventChannel = new EventChannel(EventChannel::TTL, num_of_ttl_channels, sizeof(uint8), CoreServices::getGlobalSampleRate(), this);
	ttlEventChannel->setIdentifier("TTL_deep.event");

	// set array
	eventChannelArray.add(ttlEventChannel);
	*/
}


void MultiDetector::sendTTLEvent1(uint64 bufferTs, int bufferNumSamples, int sample_index, int eventChannel) {

	/*
	// Send on event
	juce::uint8 ttlDataOn = 1 << eventChannel;
	int sampleNumOn = std::max(sample_index, 0);
	juce::int64 eventTsOn = bufferTs + sampleNumOn;
	TTLEventPtr eventOn = TTLEvent::createTTLEvent(ttlEventChannel, eventTsOn, &ttlDataOn, sizeof(juce::uint8), eventChannel);
	addEvent(ttlEventChannel, eventOn, sampleNumOn);

	// Send off event
	juce::uint8 ttlDataOff = 0;
	int sampleNumOff = sampleNumOn + pulseDurationSamples;
	juce::int64 eventTsOff = bufferTs + sampleNumOff;
	TTLEventPtr eventOff = TTLEvent::createTTLEvent(ttlEventChannel, eventTsOff, &ttlDataOff, sizeof(juce::uint8), eventChannel);


	//std::cout << eventTsOn << "  " << eventTsOff << std::endl;

	// Check if turn off event occurs during the actual buffer
	if (sampleNumOff <= bufferNumSamples) {
		// If it does, simply add the turn off event
		addEvent(ttlEventChannel, eventOff, sampleNumOff);
	}
	else {
		// if not, saves it for the next buffer
		turnoffEvent1 = eventOff;
	}
	*/
}

void MultiDetector::sendTTLEvent2(uint64 bufferTs, int bufferNumSamples, int sample_index, int eventChannel) {
	// Send on event

	/*
	juce::uint8 ttlDataOn = 1 << eventChannel;
	int sampleNumOn = std::max(sample_index, 0);
	juce::int64 eventTsOn = bufferTs + sampleNumOn;
	TTLEventPtr eventOn = TTLEvent::createTTLEvent(ttlEventChannel, eventTsOn, &ttlDataOn, sizeof(juce::uint8), eventChannel);
	addEvent(ttlEventChannel, eventOn, sampleNumOn);

	// Send off event
	juce::uint8 ttlDataOff = 0;
	int sampleNumOff = sampleNumOn + pulseDurationSamples;
	juce::int64 eventTsOff = bufferTs + sampleNumOff;
	TTLEventPtr eventOff = TTLEvent::createTTLEvent(ttlEventChannel, eventTsOff, &ttlDataOff, sizeof(juce::uint8), eventChannel);


	//std::cout << eventTsOn << "  " << eventTsOff << std::endl;

	// Check if turn off event occurs during the actual buffer
	if (sampleNumOff <= bufferNumSamples) {
		// If it does, simply add the turn off event
		addEvent(ttlEventChannel, eventOff, sampleNumOff);
	}
	else {
		// if not, saves it for the next buffer
		turnoffEvent2 = eventOff;
	}
	*/
}




bool MultiDetector::setFile(String fullpath) {

	modelPath = fullpath;

	if (tf_functions::load_session(modelPath.toStdString().c_str(), &graph, &session) == 0) {
		modelLoaded = true;

		// serving_default_conv1d_input
		// serving_default_input_1
		TF_Operation* input_op = TF_GraphOperationByName(graph, ("serving_default_"+inputLayer).toStdString().c_str());
		input = TF_Output{ input_op, 0 };
		if (input.oper == nullptr) {
			printf("Can't init input_op\n");
			return false;
		}

		printf("init input_op\n");

		TF_Operation* output_op = TF_GraphOperationByName(graph, "StatefulPartitionedCall");
		output = TF_Output{ output_op, 0 };
		if (output.oper == nullptr) {
			printf("Can't init output_op\n");
			return false;
		}

		printf("init output_op\n");
	}
	else {
		return false;
	}


	printf("%s\n", modelPath.toStdString().c_str());

	modelLoaded = true;

	return true;
}


void MultiDetector::setPredictBufferSize(float newPredictBufferSize) {
	predictBufferSize = int(std::floor(newPredictBufferSize * downsampledSamplingRate));
}

void MultiDetector::setStride(float newStride) {
	effectiveStride = int(std::floor(newStride * downsampledSamplingRate));
}

void MultiDetector::setTimeout(int newTimeout) {
	timeout = newTimeout;
	timeoutSamples = int(std::floor(timeout * samplingRate / 1000.0f));
	timeoutDownsampled = int(std::floor(timeoutSamples / downsampleFactor));
}

void MultiDetector::setPulseDuration(int newPulseDuration) {
	pulseDuration = newPulseDuration;
	pulseDurationSamples = int(std::ceil(pulseDuration * samplingRate / 1000.0f));
}

void MultiDetector::setCalibrationTime(float newCalibrationTime) {
	calibrationTime = newCalibrationTime;

	for (int i = 0; i < NUM_CHANNELS; i++) {
		calibrationBuffer[i] = std::vector<float>(calibrationTime * downsampledSamplingRate);
		channelsMeans[i] = 0.;
	}

	isCalibration = true;
	elapsedCalibration = 0;
}

void MultiDetector::setThreshold1(float newThreshold) {
	threshold1 = newThreshold;

	if (threshold1 < 0) {
		thresholdSign1 = -1;
	} else {
		thresholdSign1 = 1;
	}
}

void MultiDetector::setThreshold2(float newThreshold) {
	threshold2 = newThreshold;

	if (threshold2 < 0) {
		thresholdSign2 = -1;
	} else {
		thresholdSign2 = 1;
	}
}

void MultiDetector::setInputLayer(const String& newInputLayer) {
	inputLayer = newInputLayer;
}

void MultiDetector::setChannel1(int channel) {
	channel1 = channel;
}

void MultiDetector::setChannel2(int channel) {
	channel2 = channel;
}

void MultiDetector::setThrDrift(float newThrDrift) {
	thrDrift = newThrDrift;
}

float MultiDetector::getPredictBufferSize() {
	return predictBufferSize / downsampledSamplingRate;
}

float MultiDetector::getStride() {
	return effectiveStride / downsampledSamplingRate;
}

int MultiDetector::getTimeout() {
	return timeout;
}

int MultiDetector::getPulseDuration() {
	return pulseDuration;
}

float MultiDetector::getCalibrationTime() {
	return calibrationTime;
}

float MultiDetector::getThreshold1() {
	return threshold1;
}

float MultiDetector::getThreshold2() {
	return threshold2;
}

String MultiDetector::getInputLayer() {
	return inputLayer;
}

float MultiDetector::getThrDrift() {
	return thrDrift;
}

float MultiDetector::calculateMean(std::vector<float> data) {
	float sum = 0;
	int length = data.size();

	for (int i = 0; i < length; i++) {
		sum += data[i];
	}

	return sum / length;
}

float MultiDetector::calculateStd(std::vector<float> data, float mean) {
	float sum = 0;
	int length = data.size();

	for (int i = 0; i < length; i++) {
		float aux = data[i] - mean;
		sum += (aux * aux);
	}

	return sqrt(sum / length);
}

void  MultiDetector::pushMeanStd(double x, int chan) {
	// See Knuth TAOCP vol 2, 3rd edition, page 232
	if (elapsedCalibration == 1) {
		channelsOldMeans[chan] = channelsNewMeans[chan] = x;
		channelsOldStds[chan] = 0.0;
	}
	else {
		channelsNewMeans[chan] = channelsOldMeans[chan] + (x - channelsOldMeans[chan]) / elapsedCalibration;
		channelsNewStds[chan] = channelsOldStds[chan] + (x - channelsOldMeans[chan]) * (x - channelsNewMeans[chan]);

		// set up for next iteration
		channelsOldMeans[chan] = channelsNewMeans[chan];
		channelsOldStds[chan] = channelsNewStds[chan];
	}
}

double MultiDetector::getMean(int chan) {
	return channelsNewMeans[chan];
}

double MultiDetector::getStd(int chan) {
	double s = sqrt(channelsNewStds[chan] / (elapsedCalibration - 1));
	return (s > 0.0) ? s : 1.0;
}
