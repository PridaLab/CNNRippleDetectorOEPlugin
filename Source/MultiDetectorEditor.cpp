#include "MultiDetectorEditor.h"
#include "MultiDetector.h"

CustomTextBoxParameterEditor::CustomTextBoxParameterEditor(Parameter* param) : ParameterEditor(param)
{

    jassert(param->getType() == Parameter::FLOAT_PARAM
        || param->getType() == Parameter::INT_PARAM
        || param->getType() == Parameter::STRING_PARAM);

    parameterNameLabel = std::make_unique<Label>("Parameter name", param->getName());
    Font labelFont = Font("Silkscreen", "Regular", 10);
    int labelWidth = 120; //labelFont.getStringWidth(param->getName());
    parameterNameLabel->setFont(labelFont);
    parameterNameLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(parameterNameLabel.get());

    if(param->getType() == Parameter::FLOAT_PARAM)
        valueTextBox = std::make_unique<Label>("Parameter value", String(float(param->getValue())));
    else
        valueTextBox = std::make_unique<Label>("Parameter value", param->getValue().toString());

    valueTextBox->setFont(Font("CP Mono", "Plain", 15));
    valueTextBox->setName(param->getProcessor()->getName() + " (" + String(param->getProcessor()->getNodeId()) + ") - " + param->getName());
    valueTextBox->setColour(Label::textColourId, Colours::white);
    valueTextBox->setColour(Label::backgroundColourId, Colours::grey);
    valueTextBox->setEditable(true);
    valueTextBox->addListener(this);
    valueTextBox->setTooltip(param->getDescription());
    addAndMakeVisible(valueTextBox.get());
    
    finalWidth = std::max(labelWidth, 80);

    setBounds(0, 0, labelWidth, 40);
}

void CustomTextBoxParameterEditor::labelTextChanged(Label* label)
{
    if(param->getType() == Parameter::FLOAT_PARAM)
        param->setNextValue(label->getText().getFloatValue());
    else
        param->setNextValue(label->getText());
}

void CustomTextBoxParameterEditor::updateView()
{
    
    if (param != nullptr)
    {
        valueTextBox->setEditable(true);

        if(param->getType() == Parameter::FLOAT_PARAM)
            valueTextBox->setText(String(float(param->getValue())), dontSendNotification);
        else
            valueTextBox->setText(param->getValue().toString(), dontSendNotification);
    }
    else 
	{
        valueTextBox->setEditable(false);
    }

}

void CustomTextBoxParameterEditor::resized()
{
    parameterNameLabel->setBounds(0, 0, 120, 18);
    valueTextBox->setBounds(5,20, 60, 18);
}

MultiDetectorEditor::MultiDetectorEditor(GenericProcessor* parentNode)
	: GenericEditor(parentNode)
{

    rippleDetector = (MultiDetector*)parentNode;

    fileButton = std::make_unique<UtilityButton>("Load Model", titleFont);
    fileButton->addListener(this);
    fileButton->setRadius(3.0f);
    fileButton->setTooltip("Load a model file");
    fileButton->setBounds(10, 25, 80, 20);
    addAndMakeVisible(fileButton.get());

    fileNameLabel = std::make_unique<Label>("File name", "No file loaded");
    fileNameLabel->setFont(Font("Silkscreen", "Regular", 10));
    fileNameLabel->setBounds(100, 25, 200, 20);
    addAndMakeVisible(fileNameLabel.get());

    Parameter* param = getProcessor()->getParameter("pulse_duration");
    addCustomParameterEditor(new CustomTextBoxParameterEditor(param), 10, 45);

    param = getProcessor()->getParameter("timeout");
    addCustomParameterEditor(new CustomTextBoxParameterEditor(param), 10, 85);

    param = getProcessor()->getParameter("calibration_time");
    addCustomParameterEditor(new CustomTextBoxParameterEditor(param), 130, 45);

    param = getProcessor()->getParameter("threshold");
    addCustomParameterEditor(new CustomTextBoxParameterEditor(param), 130, 85);

    param = getProcessor()->getParameter("drift");
    addCustomParameterEditor(new CustomTextBoxParameterEditor(param), 250, 45);

    addComboBoxParameterEditor("output", 250, 85);

	lastFilePath = CoreServices::getDefaultUserSaveDirectory();
    // More extensions can be added an separated with semi-collons
    // i.e. "*.pb;*.txt;*.exe"
    supportedFileExtensions = "*.pb"; 

    int fontSize = 15;
    desiredWidth = 330;


	// /* ------------- Top row (File selector) ------------- */
	// int xPos = 12;
    // int yPos = 26;

	// fileButton = new UtilityButton("F:", Font ("Small Text", 13, Font::plain));
    // //fileButton->addListener(this);
    // fileButton->setBounds(xPos, yPos, 20, fontSize);
    // addAndMakeVisible(fileButton);

    // fileNameLabel = createLabel("FileNameLabel", "No file selected.", {xPos + 20, yPos, 140, fontSize});
    // addAndMakeVisible(fileNameLabel);

    // /*
    // windowSizeLabel = createLabel("windowSizeLabel", "Window size (s):", {xPos + 325, yPos, 140, fontSize});
    // addAndMakeVisible(windowSizeLabel);

    // windowSizeText = createTextField("windowSizeText", String(rippleDetector->getPredictBufferSize()), "Prediction window size in seconds", {xPos + 325+125, yPos, 60, fontSize});
    // addAndMakeVisible(windowSizeText);

    // strideLabel = createLabel("strideLabel", "Stride (s):", {xPos + 325, yPos + 20, 140, fontSize});
    // addAndMakeVisible(strideLabel);

    // strideText = createTextField("strideText", String(rippleDetector->getStride()), "Stride in seconds", {xPos + 325+125, yPos + 20, 60, fontSize});
    // addAndMakeVisible(strideText);
    // */


    // /* ------------- Middle row (Pulse duration) ------------- */
    // yPos += 20;

    // pulseDurationLabel = createLabel("PulseDurationLabel", "Pulse duration (ms):", {xPos, yPos, 140, fontSize});
    // addAndMakeVisible(pulseDurationLabel);

    // pulseDurationText = createTextField("PulseDurationText", String(rippleDetector->getPulseDuration()), "Duration of the TTL pulse", {xPos + 10, yPos + 20, 50, fontSize});
    // addAndMakeVisible(pulseDurationText);

    // calibrationTimeLabel = createLabel("calibrationTimeLabel", "Calibration time (s):", { xPos + 150, yPos, 140, fontSize });
    // addAndMakeVisible(calibrationTimeLabel);

    // calibrationTimeText = createTextField("calibrationTimeText", String(rippleDetector->getCalibrationTime()), "Duration of calibration time", { xPos + 150 + 10, yPos + 20, 50, fontSize });
    // addAndMakeVisible(calibrationTimeText);

    // thrDriftLabel = createLabel("thrDriftLabel", "Drift (SD):", { xPos + 315, yPos, 140, fontSize });
    // addAndMakeVisible(thrDriftLabel);

    // thrDriftText = createTextField("thrDriftText", String(rippleDetector->getThrDrift()), "Drift prevention threshold (standard deviations)", { xPos + 315 + 10, yPos + 20, 50, fontSize });
    // addAndMakeVisible(thrDriftText);



    // /*inputLayerText = createTextField("inputLayerText", rippleDetector->getInputLayer(), "inputLayer", { xPos + 400, yPos + 20, 200, fontSize });
    // addAndMakeVisible(inputLayerText);*/



    // /* ------------- Bottom row (Timeout duration) ------------- */
    // yPos += 40;
    
    // timeoutLabel = createLabel("TimeoutLabel", "Timeout (ms):", {xPos, yPos, 140, fontSize});
    // addAndMakeVisible(timeoutLabel);

    // timeoutText = createTextField("TimeoutText", String(rippleDetector->getTimeout()), "Minimum time between events", {xPos + 10, yPos + 20, 50, fontSize});
    // addAndMakeVisible(timeoutText);

    // thresholdLabel1 = createLabel("thresholdLabel1", "Threshold:", { xPos + 150, yPos, 140, fontSize });
    // addAndMakeVisible(thresholdLabel1);

    // thresholdText1 = createTextField("thresholdText1", String(rippleDetector->getThreshold1()), "Probability threshold", { xPos + 150 + 10, yPos + 20, 50, fontSize });
    // addAndMakeVisible(thresholdText1);

    // /*thresholdLabel2 = createLabel("thresholdLabel2", "Thresh 2:", { xPos + 250, yPos, 140, fontSize });
    // addAndMakeVisible(thresholdLabel2);

    // thresholdText2 = createTextField("thresholdText2", String(rippleDetector->getThreshold2()), "Probability threshold", { xPos + 250, yPos + 20, 50, fontSize });
    // addAndMakeVisible(thresholdText2);*/

    // outLabel1 = createLabel("outLabel1", "Output:", { xPos + 315, yPos, 140, fontSize });
    // addAndMakeVisible(outLabel1);

    // outSelector1 = new ComboBox("Out First Channel");
    // for (int chan = 1; chan <= 8; chan++)
    //     outSelector1->addItem(String(chan), chan);
    // outSelector1->addItem(" ", 9);
    // outSelector1->setTooltip("TTL channel 1");
    // outSelector1->setBounds(xPos + 315 + 10, yPos + 20, 40, fontSize);
    // outSelector1->addListener(this);
    // addAndMakeVisible(outSelector1);

    // /*outLabel2 = createLabel("outLabel2", "Out 2:", { xPos + 500, yPos, 140, fontSize });
    // addAndMakeVisible(outLabel2);

    // outSelector2 = new ComboBox("Out Second Channel");
    // for (int chan = 1; chan <= 8; chan++)
    //     outSelector2->addItem(String(chan), chan);
    // outSelector2->addItem(" ", 9);
    // outSelector2->setTooltip("TTL channel 2");
    // outSelector2->setBounds(xPos + 500, yPos + 20, 40, fontSize);
    // outSelector2->addListener(this);
    // addAndMakeVisible(outSelector2);*/
}

/*
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
*/

/**
The listener methods that reacts to the button click. The same method is called for all buttons
on the editor, so the button variable, which cointains a pointer to the button that called the method
has to be checked to know which function to perform.
*/
void MultiDetectorEditor::buttonClicked(Button*)
{
    FileChooser chooseFileReaderFile ("Please select the file you want to load...", lastFilePath, supportedFileExtensions);

    if (chooseFileReaderFile.browseForFileToOpen()) {
        // Use the selected file
        setFile(chooseFileReaderFile.getResult().getFullPathName());
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

        //setEnabledState(true);
    }
    else {
        //clearEditor();
        fileNameLabel->setText("No file selected.", dontSendNotification);
    }

    CoreServices::updateSignalChain(this);
    repaint();
}




/*
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
    } else if (labelThatHasChanged == thrDriftText) {
        float newThrDrift;

        if (updateFloatLabel(labelThatHasChanged, -50., 50., rippleDetector->getThrDrift(), &newThrDrift)) {
            rippleDetector->setThrDrift(newThrDrift);
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
*/


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


