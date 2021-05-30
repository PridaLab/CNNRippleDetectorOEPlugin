#ifndef __MULTIDETECTOREDITOR_H_DEFINED__
#define __MULTIDETECTOREDITOR_H_DEFINED__

#include <EditorHeaders.h>
#include "MultiDetector.h"
/**

  User interface for the MultiDetector processor.

  @see MultiDetector

*/


class MultiDetectorEditor : public GenericEditor, public Label::Listener, public ComboBox::Listener
{
public:
    MultiDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    ~MultiDetectorEditor();

    void labelTextChanged(Label* labelThatHasChanged) override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
    void buttonEvent(Button* button) override;

private:
	MultiDetectorSpace::MultiDetector * rippleDetector;

	File lastFilePath;
  String supportedFileExtensions;

	ScopedPointer<UtilityButton> fileButton;
  ScopedPointer<Label> fileNameLabel;

  ScopedPointer<Label> windowSizeLabel;
  ScopedPointer<Label> windowSizeText;
  ScopedPointer<Label> strideLabel;
  ScopedPointer<Label> strideText;

  ScopedPointer<Label> pulseDurationLabel;
  ScopedPointer<Label> timeoutLabel;
  ScopedPointer<Label> calibrationTimeLabel;
  ScopedPointer<Label> pulseDurationText;
  ScopedPointer<Label> timeoutText;
  ScopedPointer<Label> calibrationTimeText;
  
  ScopedPointer<Label> inputLayerText;
  
  ScopedPointer<Label> thresholdLabel1;
  ScopedPointer<Label> thresholdLabel2;
  ScopedPointer<Label> thresholdText1;
  ScopedPointer<Label> thresholdText2;

  ScopedPointer<Label> outLabel1;
  ScopedPointer<Label> outLabel2;
  ScopedPointer<ComboBox> outSelector1;
  ScopedPointer<ComboBox> outSelector2;

  Label * createLabel(const String& name, const String& text, juce::Rectangle<int> bounds);
  Label * createTextField(const String& name, const String& initialValue, const String& tooltip, juce::Rectangle<int> bounds);

  bool updateIntLabel(Label * label, int min, int max, int defaultValue, int * out);
  bool updateFloatLabel(Label* label, float min, float max, float defaultValue, float* out);
  bool updateStringLabel(Label* label, String defaultValue, String& out);

  void setFile(String file);


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiDetectorEditor);

};


#endif 
