#include "MultiDetectorEditor.h"
#include "MultiDetector.h"

MultiDetectorEditor::MultiDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
	: GenericEditor(parentNode, useDefaultParameterEditors)
	, rippleDetector   (static_cast<MultiDetectorSpace::MultiDetector*> (parentNode))
{
	lastFilePath = CoreServices::getDefaultUserSaveDirectory();
    // More extensions can be added an separated with semi-collons
    // i.e. "*.pb;*.txt;*.exe"
    supportedFileExtensions = "*.pb"; 

    int fontSize = 15;
    desiredWidth = 650;


	/* ------------- Top row (File selector) ------------- */
	int xPos = 12;
    int yPos = 26;

	fileButton = new UtilityButton("F:", Font ("Small Text", 13, Font::plain));
    fileButton->addListener(this);
    fileButton->setBounds(xPos, yPos, 20, fontSize);
    addAndMakeVisible(fileButton);

    fileNameLabel = createLabel("FileNameLabel", "No file selected.", {xPos + 20, yPos, 140, fontSize});
    addAndMakeVisible(fileNameLabel);

    windowSizeLabel = createLabel("windowSizeLabel", "Window size (s):", {xPos + 400, yPos, 140, fontSize});
    addAndMakeVisible(windowSizeLabel);

    windowSizeText = createTextField("windowSizeText", String(rippleDetector->getPredictBufferSize()), "Prediction window size in seconds", {xPos + 525, yPos, 60, fontSize});
    addAndMakeVisible(windowSizeText);

    strideLabel = createLabel("strideLabel", "Stride (s):", {xPos + 400, yPos + 20, 140, fontSize});
    addAndMakeVisible(strideLabel);

    strideText = createTextField("strideText", String(rippleDetector->getStride()), "Stride in seconds", {xPos + 525, yPos + 20, 60, fontSize});
    addAndMakeVisible(strideText);


    /* ------------- Middle row (Pulse duration) ------------- */
    yPos += 20;

    pulseDurationLabel = createLabel("PulseDurationLabel", "Pulse duration (ms):", {xPos, yPos, 140, fontSize});
    addAndMakeVisible(pulseDurationLabel);

    pulseDurationText = createTextField("PulseDurationText", String(rippleDetector->getPulseDuration()), "Duration of the TTL pulse", {xPos, yPos + 20, 50, fontSize});
    addAndMakeVisible(pulseDurationText);

    calibrationTimeLabel = createLabel("calibrationTimeLabel", "Calibration time (s):", { xPos + 150, yPos, 140, fontSize });
    addAndMakeVisible(calibrationTimeLabel);

    calibrationTimeText = createTextField("calibrationTimeText", String(rippleDetector->getCalibrationTime()), "Duration of calibration time", { xPos + 150, yPos + 20, 50, fontSize });
    addAndMakeVisible(calibrationTimeText);



    /*inputLayerText = createTextField("inputLayerText", rippleDetector->getInputLayer(), "inputLayer", { xPos + 400, yPos + 20, 200, fontSize });
    addAndMakeVisible(inputLayerText);*/



    /* ------------- Bottom row (Timeout duration) ------------- */
    yPos += 40;
    
    timeoutLabel = createLabel("TimeoutLabel", "Timeout (ms):", {xPos, yPos, 140, fontSize});
    addAndMakeVisible(timeoutLabel);

    timeoutText = createTextField("TimeoutText", String(rippleDetector->getTimeout()), "Minimum time between events", {xPos, yPos + 20, 50, fontSize});
    addAndMakeVisible(timeoutText);

    thresholdLabel1 = createLabel("thresholdLabel1", "Thresh 1:", { xPos + 150, yPos, 140, fontSize });
    addAndMakeVisible(thresholdLabel1);

    thresholdText1 = createTextField("thresholdText1", String(rippleDetector->getThreshold1()), "Probability threshold", { xPos + 150, yPos + 20, 50, fontSize });
    addAndMakeVisible(thresholdText1);

    /*thresholdLabel2 = createLabel("thresholdLabel2", "Thresh 2:", { xPos + 250, yPos, 140, fontSize });
    addAndMakeVisible(thresholdLabel2);

    thresholdText2 = createTextField("thresholdText2", String(rippleDetector->getThreshold2()), "Probability threshold", { xPos + 250, yPos + 20, 50, fontSize });
    addAndMakeVisible(thresholdText2);*/

    outLabel1 = createLabel("outLabel1", "Out 1:", { xPos + 400, yPos, 140, fontSize });
    addAndMakeVisible(outLabel1);

    outSelector1 = new ComboBox("Out First Channel");
    for (int chan = 1; chan <= 8; chan++)
        outSelector1->addItem(String(chan), chan);
    outSelector1->addItem(" ", 9);
    outSelector1->setTooltip("TTL channel 1");
    outSelector1->setBounds(xPos + 400, yPos + 20, 40, fontSize);
    outSelector1->addListener(this);
    addAndMakeVisible(outSelector1);

    /*outLabel2 = createLabel("outLabel2", "Out 2:", { xPos + 500, yPos, 140, fontSize });
    addAndMakeVisible(outLabel2);

    outSelector2 = new ComboBox("Out Second Channel");
    for (int chan = 1; chan <= 8; chan++)
        outSelector2->addItem(String(chan), chan);
    outSelector2->addItem(" ", 9);
    outSelector2->setTooltip("TTL channel 2");
    outSelector2->setBounds(xPos + 500, yPos + 20, 40, fontSize);
    outSelector2->addListener(this);
    addAndMakeVisible(outSelector2);*/
}

MultiDetectorEditor::~MultiDetectorEditor()
{
}


Label * MultiDetectorEditor::createTextField (const String& name, const String& initialValue, const String& tooltip, juce::Rectangle<int> bounds)
{
    Label* textField = new Label(name, initialValue);
    textField->setEditable(true);
    textField->addListener(this);
    textField->setBounds(bounds);
    textField->setColour(Label::backgroundColourId, Colours::grey);
    textField->setColour(Label::textColourId, Colours::white);
    if (tooltip.length() > 0)
    {
        textField->setTooltip(tooltip);
    }

    return textField;
}

Label * MultiDetectorEditor::createLabel (const String& name, const String& text, juce::Rectangle<int> bounds)
{
    Label* label = new Label(name, text);
    label->setBounds(bounds);
    label->setFont(Font("Small Text", 12, Font::plain));
    label->setColour(Label::textColourId, Colours::darkgrey);

    return label;
}

/**
The listener methods that reacts to the button click. The same method is called for all buttons
on the editor, so the button variable, which cointains a pointer to the button that called the method
has to be checked to know which function to perform.
*/
void MultiDetectorEditor::buttonEvent(Button* button)
{
	if (button == fileButton) {
        FileChooser chooseFileReaderFile ("Please select the file you want to load...", lastFilePath, supportedFileExtensions);

        if (chooseFileReaderFile.browseForFileToOpen()) {
            // Use the selected file
            setFile(chooseFileReaderFile.getResult().getFullPathName());
        }
	}

}


void MultiDetectorEditor::setFile (String file)
{
    File fileToRead(file);
    lastFilePath = fileToRead.getParentDirectory();

    String fileFullName = fileToRead.getFullPathName();
    String filePath = lastFilePath.getFullPathName();
    
    if (rippleDetector->setFile(filePath)) {
        fileNameLabel->setText(fileFullName, dontSendNotification);

        setEnabledState(true);
    }
    else {
        //clearEditor();
        fileNameLabel->setText("No file selected.", dontSendNotification);
    }

    CoreServices::updateSignalChain(this);
    repaint();
}





void MultiDetectorEditor::labelTextChanged(Label * labelThatHasChanged)
{
    int int_max = 2147483647;

    if (labelThatHasChanged == timeoutText) {
        int newTimeout;

        if (updateIntLabel(labelThatHasChanged, 0, int_max, rippleDetector->getTimeout(), &newTimeout)) {
            rippleDetector->setTimeout(newTimeout);
        }
    }
    else if (labelThatHasChanged == pulseDurationText) {
        int newPulseDuration;

        if (updateIntLabel(labelThatHasChanged, 0, int_max, rippleDetector->getPulseDuration(), &newPulseDuration)) {
            rippleDetector->setPulseDuration(newPulseDuration);
        }
    } else if (labelThatHasChanged == calibrationTimeText) {
        float newCalibrationTime;

        if (updateFloatLabel(labelThatHasChanged, 0, 10000., rippleDetector->getCalibrationTime(), &newCalibrationTime)) {
            rippleDetector->setCalibrationTime(newCalibrationTime);
        }
    } else if (labelThatHasChanged == thresholdText1) {
        float newThreshold;

        if (updateFloatLabel(labelThatHasChanged, -50., 50., rippleDetector->getThreshold1(), &newThreshold)) {
            rippleDetector->setThreshold1(newThreshold);
        }
    } else if (labelThatHasChanged == thresholdText2) {
        float newThreshold;

        if (updateFloatLabel(labelThatHasChanged, -50., 50., rippleDetector->getThreshold2(), &newThreshold)) {
            rippleDetector->setThreshold2(newThreshold);
        }
    } else if (labelThatHasChanged == inputLayerText) {
        String newInputLayer;

        if (updateStringLabel(labelThatHasChanged, rippleDetector->getInputLayer(), newInputLayer)) {
            rippleDetector->setInputLayer(newInputLayer);
        }
    } else if (labelThatHasChanged == windowSizeText) {
        float newWindowSize;

        if (updateFloatLabel(labelThatHasChanged, -50., 50., rippleDetector->getPredictBufferSize(), &newWindowSize)) {
            rippleDetector->setPredictBufferSize(newWindowSize);
        }
    } else if (labelThatHasChanged == strideText) {
        float newStride;

        if (updateFloatLabel(labelThatHasChanged, -50., 50., rippleDetector->getStride(), &newStride)) {
            rippleDetector->setStride(newStride);
        }
    }
}


void MultiDetectorEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == outSelector1) 
    {
        int idx = static_cast<int>(outSelector1->getSelectedId());
        if (outSelector1->getSelectedId() > 8) 
            idx = 0;

        rippleDetector->setChannel1(idx - 1);
    } 

    else if (comboBoxThatHasChanged == outSelector2) 
    {
        int idx = static_cast<int>(outSelector2->getSelectedId());
        if (outSelector2->getSelectedId() > 8) 
            idx = 0;

        rippleDetector->setChannel2(idx - 1);
    }
}


bool MultiDetectorEditor::updateIntLabel(Label* label, int min, int max, int defaultValue, int* out)
{
    const String& in = label->getText();
    int parsedInt;
    try
    {
        parsedInt = std::atoi(in.toRawUTF8());
    }
    catch (const std::logic_error&)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    *out = jmax(min, jmin(max, parsedInt));

    label->setText(String(*out), dontSendNotification);
    return true;
}


bool MultiDetectorEditor::updateFloatLabel(Label* label, float min, float max, float defaultValue, float* out)
{
    const String& in = label->getText();
    float parsedFloat;
    try
    {
        parsedFloat = std::atof(in.toRawUTF8());
    }
    catch (const std::logic_error&)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    *out = jmax(min, jmin(max, parsedFloat));

    label->setText(String(*out), dontSendNotification);
    return true;
}


bool MultiDetectorEditor::updateStringLabel(Label* label, String defaultValue, String& out)
{
    const String& in = label->getText();
    String parsedString;
    try
    {
        parsedString = in.toRawUTF8();
    }
    catch (const std::logic_error&)
    {
        label->setText(defaultValue, dontSendNotification);
        return false;
    }

    out = in;
    label->setText(in, dontSendNotification);
    return true;
}


