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

	addSelectedChannelsParameter(
		Parameter::STREAM_SCOPE,
		"CNN_Input",
		"The 8 continuous channels to use as input into the CNN",
		8
	);

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
		 0.5, 	//default
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

	inputLayer = "conv1d_input";

	modelLoaded = false;
	isEnabled = modelLoaded;

	/*
	nextSampleEnable = 0;
	globalSample = 0;
	forwardSamples = 0;

	threshold1 = 0.5;
	thresholdSign1 = 1;
	threshold2 = 0.5;
	thresholdSign2 = 1;
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

	if (modelLoaded)
		tf_functions::delete_session(session);

}

void MultiDetector::updateSettings()
{

	settings.update(getDataStreams());

	for (auto stream : getDataStreams())
	{

        /* Check if NUM_CHANNELS input channels have already been set.
           If not, default to the first NUM_CHANNELS available channels */
        if (stream->getParameter("CNN_Input")->getValue().size() != NUM_CHANNELS) {

            settings[stream->getStreamId()]->inputChannels.clear();
            var selectedChannels;
            for (int i = 0; i < NUM_CHANNELS; i++)
            {
                selectedChannels.append(i);
                settings[stream->getStreamId()]->inputChannels.add(i);
            }
            stream->getParameter("CNN_Input")->setNextValue(selectedChannels);

        }

		settings[stream->getStreamId()]->pulseDuration = 0;
		settings[stream->getStreamId()]->pulseDurationSamples = 0;
		settings[stream->getStreamId()]->calibrationTime = 0;
		settings[stream->getStreamId()]->elapsedCalibrationPoints = 0;
		settings[stream->getStreamId()]->isCalibrating = true;
		settings[stream->getStreamId()]->drift = 0;
		settings[stream->getStreamId()]->timeout = 0;
		settings[stream->getStreamId()]->timeoutSamples = 0;
		settings[stream->getStreamId()]->timeoutDownsampled = 0;
		settings[stream->getStreamId()]->threshold = 0.0f;
		settings[stream->getStreamId()]->thresholdSign = 0;
		settings[stream->getStreamId()]->outputChannel = 0;

		settings[stream->getStreamId()]->roundBufferReadIndex = 0;
		settings[stream->getStreamId()]->roundBufferWriteIndex = 0;
		settings[stream->getStreamId()]->roundBufferNumElements = 0;

		settings[stream->getStreamId()]->predictBuffer = std::vector<float>(predictBufferSize * NUM_CHANNELS);
		settings[stream->getStreamId()]->predictBufferSum = std::vector<float>(predictBufferSize);
        
        settings[stream->getStreamId()]->globalSample = 0;
        settings[stream->getStreamId()]->calibrationBuffer = std::vector<std::vector<float>>(NUM_CHANNELS);

        settings[stream->getStreamId()]->channelsOldStds = std::vector<double>(NUM_CHANNELS);
        settings[stream->getStreamId()]->channelsNewStds = std::vector<double>(NUM_CHANNELS);
        settings[stream->getStreamId()]->channelsStds = std::vector<double>(NUM_CHANNELS);

        settings[stream->getStreamId()]->channelsOldMeans = std::vector<double>(NUM_CHANNELS);
        settings[stream->getStreamId()]->channelsNewMeans = std::vector<double>(NUM_CHANNELS);
        settings[stream->getStreamId()]->channelsMeans = std::vector<double>(NUM_CHANNELS);

        settings[stream->getStreamId()]->sinceLast = effectiveStride;
		settings[stream->getStreamId()]->nextSampleEnable = 0;
		settings[stream->getStreamId()]->downsampleFactor = stream->getSampleRate() / downsampledSamplingRate;

		parameterValueChanged(stream->getParameter("CNN_Input"));
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
		settings[stream->getStreamId()]->turnoffEvent1 = nullptr;


		for (int i = 0; i < NUM_CHANNELS; i++) {
			settings[stream->getStreamId()]->channelsMeans[i] = 0.;
			settings[stream->getStreamId()]->predictBufferSum[i] = 0.;
		}
	}

	isEnabled = modelLoaded;

}


AudioProcessorEditor* MultiDetector::createEditor()
{
	 editor = std::make_unique<MultiDetectorEditor>(this);

	return editor.get();
}

void MultiDetector::parameterValueChanged(Parameter* param)
{

	String paramName = param->getName();
	int streamId = param->getStreamId();

	if (paramName.equalsIgnoreCase("CNN_Input"))
	{
        Array<var>* array = param->getValue().getArray();
        if (array->size() == NUM_CHANNELS)
        {
            settings[streamId]->inputChannels.clear();

            for (int i = 0; i < NUM_CHANNELS; i++)
            {
                int localIndex = int(array->getReference(i));
                int globalIndex = getDataStream(param->getStreamId())->getContinuousChannels()[localIndex]->getGlobalIndex();
                settings[streamId]->inputChannels.add(globalIndex);
            }

            settings[streamId]->isCalibrating = true;
			settings[streamId]->elapsedCalibrationPoints = 0;
        }	
	}
	else if (paramName.equalsIgnoreCase("pulse_duration"))
	{
		settings[streamId]->pulseDuration = param->getValue();
		settings[streamId]->pulseDurationSamples = int(std::ceil(settings[streamId]->pulseDuration * getDataStream(streamId)->getSampleRate() / 1000.0f));
	}
	else if (paramName.equalsIgnoreCase("timeout"))
	{
		settings[streamId]->timeout = param->getValue();
		settings[streamId]->timeoutSamples = int(std::floor(settings[streamId]->timeout * getDataStream(streamId)->getSampleRate() / 1000.0f));
		settings[streamId]->timeoutDownsampled = int(std::floor(settings[streamId]->timeoutSamples / settings[streamId]->downsampleFactor));
	}
	else if (paramName.equalsIgnoreCase("calibration_time"))
	{
		settings[streamId]->calibrationTime = param->getValue();
		
		settings[streamId]->isCalibrating = true;
		settings[streamId]->elapsedCalibrationPoints = 0;
	}
	else if (paramName.equalsIgnoreCase("threshold"))
	{
		settings[streamId]->threshold = param->getValue();
		if (settings[streamId]->threshold < 0) {
			settings[streamId]->thresholdSign = -1;
		} else {
			settings[streamId]->thresholdSign = 1;
		}
	}
	else if (paramName.equalsIgnoreCase("drift"))
	{
		settings[streamId]->drift = param->getValue();
	}
	else if (paramName.equalsIgnoreCase("output"))
	{
		settings[streamId]->outputChannel = (int)param->getValue() - 1;
	}

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


void MultiDetector::process(AudioBuffer<float>& buffer)
{

	for (auto stream : getDataStreams())
	{
		if ((*stream)["enable_stream"])
		{

            const uint16 streamId = stream->getStreamId();
            const int64 firstSampleNumberInBlock = getFirstSampleNumberForBlock(streamId);
            const uint32 numSamplesInBlock = getNumSamplesInBlock(streamId);
            
            unsigned int ts = (unsigned int)(1000.0f * float(firstSampleNumberInBlock) / stream->getSampleRate());
            
            // Get input channel data pointers
            const float* channelData[NUM_CHANNELS];
            for (int i = 0; i < NUM_CHANNELS; i++) {
                channelData[i] = buffer.getReadPointer(settings[streamId]->inputChannels[i]);
            }
            
            //LOGD("Stream: ", streamId, " Sample number: ", firstSampleNumberInBlock, " ts: ", ts, " data: ", *channelData[0]);
            
            // Send any TTL events stored from previous buffers
            //TTLEventPtr event = (TTLEventPtr) settings[streamId]->turnoffEvent1;
            int turnoffOffset1 = settings[streamId]->turnoffEvent1 ? juce::jmax(0, int(settings[streamId]->turnoffEvent1->getSampleNumber() - firstSampleNumberInBlock)) : -1;
            
            if (turnoffOffset1 >= 0 && turnoffOffset1 < numSamplesInBlock) {
                addEvent(settings[streamId]->turnoffEvent1, turnoffOffset1);
                settings[streamId]->turnoffEvent1 = nullptr;
            }
            
            for (int sample = 0; sample < numSamplesInBlock; sample++, settings[streamId]->globalSample++) {
                
                if ((settings[streamId]->globalSample % settings[streamId]->downsampleFactor) == 0) {
                    
                    //std::cout << settings[streamId]->globalSample << " " <<  sample << " " << channelData[0][sample] << std::endl;

                    settings[streamId]->globalSample = 0;

                    
                    
                    if (settings[streamId]->isCalibrating) {
                        
                        settings[streamId]->elapsedCalibrationPoints++;

                        for (int chan = 0; chan < NUM_CHANNELS; chan++)
                            pushMeanStd(streamId, channelData[chan][sample], chan);
                            
                        if (settings[streamId]->elapsedCalibrationPoints >= (settings[streamId]->calibrationTime * downsampledSamplingRate)) {

                            settings[streamId]->isCalibrating = false;
                            
                            for (int chan = 0; chan < NUM_CHANNELS; chan++) {
                                settings[streamId]->channelsMeans[chan] = getMean(streamId, chan);
                                settings[streamId]->channelsStds[chan] = getStd(streamId, chan);

                                //std::cout << "Chan " << chan << " mean " << settings[streamId]->channelsMeans[chan] << " std " << settings[streamId]->channelsStds[chan] << std::endl;
                            }
                        }
                        
                    }
                        
                    for (int chan = 0; chan < NUM_CHANNELS; chan++)
                        settings[streamId]->roundBuffer[settings[streamId]->roundBufferWriteIndex][chan] = channelData[chan][sample];
                        
                    settings[streamId]->roundBufferWriteIndex = (settings[streamId]->roundBufferWriteIndex + 1) % MAX_ROUND_BUFFER_SIZE;
                    if (settings[streamId]->roundBufferNumElements < predictBufferSize)
                        settings[streamId]->roundBufferNumElements++;
                    settings[streamId]->sinceLast++;
                    
                }

                if ((settings[streamId]->roundBufferNumElements >= predictBufferSize) && (settings[streamId]->sinceLast >= effectiveStride) && (sample >= settings[streamId]->nextSampleEnable)) {
                    
                    settings[streamId]->sinceLast = 0;
                    settings[streamId]->nextSampleEnable = sample + 1;
                    
                    unsigned int temporalReadIndex = settings[streamId]->roundBufferReadIndex;
                    unsigned int oldRoundBufferReadIndex = temporalReadIndex;
                    
                    settings[streamId]->roundBufferReadIndex = (oldRoundBufferReadIndex + effectiveStride) % MAX_ROUND_BUFFER_SIZE;
                    
                    if (settings[streamId]->isCalibrating) continue;
                    
                    for (int idx = 0; idx < predictBufferSize; idx++) {
                        
                        for (int chan = 0; chan < NUM_CHANNELS; chan++) {
                            
                            settings[streamId]->predictBuffer[(idx*NUM_CHANNELS) + chan] = (settings[streamId]->roundBuffer[temporalReadIndex][chan] - settings[streamId]->channelsMeans[chan]) / settings[streamId]->channelsStds[chan];
                            settings[streamId]->predictBufferSum[idx] += settings[streamId]->predictBuffer[(idx*NUM_CHANNELS) + chan];
                            
                        }
                        
                        settings[streamId]->predictBufferSum[idx] = abs(settings[streamId]->predictBufferSum[idx] / NUM_CHANNELS);
                        temporalReadIndex = (temporalReadIndex + 1) % MAX_ROUND_BUFFER_SIZE;
                    }
                    
                    if (settings[streamId]->drift > 0) {
                        
                        settings[streamId]->skipPrediction = true;
                        
                        float meanWindow = 0;
                        
                        for (int idx = 0; idx < predictBufferSize; idx++)
                        {
                            meanWindow += settings[streamId]->predictBufferSum[idx];
                            settings[streamId]->predictBufferSum[idx] = 0;
                        }
                        
                        meanWindow = meanWindow / predictBufferSize;
                        
                        if (meanWindow < settings[streamId]->drift) {
                            settings[streamId]->skipPrediction = false;
                        }
                    }
					
                    
                    if (!settings[streamId]->skipPrediction)
                    {
                        
                        TF_Tensor* input_tensor = nullptr, * output_tensor = nullptr;

                        std::vector<std::int64_t> dims = { 1, predictBufferSize, NUM_CHANNELS };
                        int num_dims = 3;
                        tf_functions::create_tensor(TF_FLOAT, dims, num_dims, settings[streamId]->predictBuffer, &input_tensor);

                        tf_functions::run_session(session, &input, &input_tensor, 1, &output, &output_tensor, 1);

                        // Check results
						if (output_tensor == NULL) return;
                        auto tensor_data = static_cast<float*>(TF_TensorData(output_tensor));
                        //std::cout << tensor_data[0] << std::endl;

                        settings[streamId]->forwardSamples = 0;
		
                        if ((settings[streamId]->thresholdSign * tensor_data[0]) >= (settings[streamId]->thresholdSign * settings[streamId]->threshold) && settings[streamId]->outputChannel >= 0) {
                            settings[streamId]->forwardSamples = 1;

                            //std::cout << tsBuffer << " " << numSamples << " " << sample << " " << tensor_data[0] << std::endl;
                            sendTTLEvent(streamId, firstSampleNumberInBlock, numSamplesInBlock, sample);
                        }

                        if (settings[streamId]->forwardSamples == 1) {
                            settings[streamId]->nextSampleEnable += settings[streamId]->timeoutSamples - 1;
                            //std::cout <<  "event" << " nextSample " << settings[streamId]->nextSampleEnable << " timeout " << settings[streamId]->timeoutSamples << std::endl;
                            // If an event has been found, next value to read will be the first upcoming window after timeout
                            settings[streamId]->roundBufferReadIndex = (oldRoundBufferReadIndex + std::max(settings[streamId]->timeoutDownsampled, effectiveStride)) % MAX_ROUND_BUFFER_SIZE;
                        }

                        tf_functions::delete_tensor(input_tensor);
                        tf_functions::delete_tensor(output_tensor);
                    }
                    
                }

				//settings[streamId]->globalSample++;
                
            }
			
			// Shift nextSampleEnable so it is relative to the next buffer
			if (settings[streamId]->nextSampleEnable - (int)numSamplesInBlock >= 0)
				settings[streamId]->nextSampleEnable = settings[streamId]->nextSampleEnable - (int)numSamplesInBlock;
			else
				settings[streamId]->nextSampleEnable = 0;
		}

	}

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


void MultiDetector::sendTTLEvent(uint64 streamId, uint64 sampleNumber, int bufferNumSamples, int sample_index) {

	// Send on event
	TTLEventPtr event = settings[streamId]->createEvent(
		settings[streamId]->outputChannel,
		sampleNumber + sample_index,
		1
	);
	addEvent(event, sample_index);

	// Send off event
	event = settings[streamId]->createEvent(
		settings[streamId]->outputChannel,
		sampleNumber + sample_index + settings[streamId]->pulseDurationSamples,
		0
	);
	//addEvent(event, sample_index + settings[streamId]->pulseDurationSamples);
	
	//std::cout << eventTsOn << "  " << eventTsOff << std::endl;

	// Check if turn off event occurs during the actual buffer
	if ((sample_index + settings[streamId]->pulseDurationSamples) <= bufferNumSamples) {
		// If it does, simply add the turn off event
		//addEvent(ttlEventChannel, eventOff, sampleNumOff);
		addEvent(event, sample_index + settings[streamId]->pulseDurationSamples);
	}
	else {
		// if not, saves it for the next buffer
		settings[streamId]->turnoffEvent1 = event;
	}
	
}


bool MultiDetector::setFile(String fullpath) {

	File path(fullpath);
	String pathString = path.getParentDirectory().getFullPathName();

	if (tf_functions::load_session(pathString.toStdString().c_str(), &graph, &session) == 0) {
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


	//printf("%s\n", modelPath.toStdString().c_str());

	modelPath = path;
	modelLoaded = true;

	isEnabled = modelLoaded;

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

void  MultiDetector::pushMeanStd(uint64 streamId, double x, int chan) {
    
	// See Knuth TAOCP vol 2, 3rd edition, page 232
	if (settings[streamId]->elapsedCalibrationPoints == 1) {
        settings[streamId]->channelsOldMeans[chan] = settings[streamId]->channelsNewMeans[chan] = x;
        settings[streamId]->channelsOldStds[chan] = 0.0;
	}
	else
    {
        settings[streamId]->channelsNewMeans[chan] = settings[streamId]->channelsOldMeans[chan] + (x - settings[streamId]->channelsOldMeans[chan]) / settings[streamId]->elapsedCalibrationPoints;
        settings[streamId]->channelsNewStds[chan] = settings[streamId]->channelsOldStds[chan] + (x - settings[streamId]->channelsOldMeans[chan]) * (x - settings[streamId]->channelsNewMeans[chan]);

		//std::cout << "Pts " << settings[streamId]->elapsedCalibrationPoints << " TChan " << chan << " x " << x << " mean " << settings[streamId]->channelsNewMeans[chan] << " std " << settings[streamId]->channelsNewStds[chan] << std::endl;

		// set up for next iteration
        settings[streamId]->channelsOldMeans[chan] = settings[streamId]->channelsNewMeans[chan];
        settings[streamId]->channelsOldStds[chan] = settings[streamId]->channelsNewStds[chan];
	}
}

double MultiDetector::getMean(uint64 streamId, int chan) {
	return settings[streamId]->channelsNewMeans[chan];
}

double MultiDetector::getStd(uint64 streamId, int chan) {
	double s = sqrt(settings[streamId]->channelsNewStds[chan] / (settings[streamId]->elapsedCalibrationPoints - 1));
	return (s > 0.0) ? s : 1.0;
}

void MultiDetector::saveCustomParametersToXml(XmlElement* xml)
{
    xml->setAttribute ("path", modelPath.getFullPathName().toStdString());
}


void MultiDetector::loadCustomParametersFromXml(XmlElement* xml)
{

    MultiDetectorEditor* editor = (MultiDetectorEditor*) getEditor();

    String path = xml->getStringAttribute("path");

    if (!File(path).exists()) {
		LOGC("Saved model path does not exist");
		return;
	}

	editor->setFile(path);

}