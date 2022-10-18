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

    addSelectedChannelsParameterEditor("CNN_Input", 10, 25);

    fileButton = std::make_unique<UtilityButton>("Load Model", titleFont);
    fileButton->addListener(this);
    fileButton->setRadius(3.0f);
    fileButton->setTooltip("Load a model file");
    fileButton->setBounds(130, 25, 80, 20);
    addAndMakeVisible(fileButton.get());

    fileNameLabel = std::make_unique<Label>("File name", "No file loaded");
    fileNameLabel->setFont(Font("Silkscreen", "Regular", 10));
    fileNameLabel->setBounds(250, 25, 200, 20);
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
    desiredWidth = 360;

}
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
    
    if (rippleDetector->setFile(fileFullName)) {
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


