#pragma once

#include <JuceHeader.h>
#include "license.h"
#include <memory>
#include <vector>

class MainComponent  : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Row
    {
        juce::String first;
        juce::String last;
        juce::String email;
        juce::String license;
    };

    void setupEditors();
    void setupButtons();
    void updateStatus(const juce::String& message, juce::Colour colour);
    bool validateInputs(juce::String& outFirst, juce::String& outLast, juce::String& outEmail);
    void updateCopyState();

    void generateLicense();
    void verifyCurrentLicense();
    void copyLicenseToClipboard();
    void loadBatchFromCsv();
    void saveBatchToCsv();

    juce::TextEditor firstEdit;
    juce::TextEditor lastEdit;
    juce::TextEditor emailEdit;
    juce::TextEditor keyOut;

    juce::TextButton btnGenerate { "Generate" };
    juce::TextButton btnVerify { "Verify" };
    juce::TextButton btnCopy { "Copy Key" };
    juce::TextButton btnBatchIn { "Batch from CSV..." };
    juce::TextButton btnSaveCsv { "Save CSV..." };

    juce::Label statusLabel;

    std::vector<Row> batchRows;
    std::unique_ptr<juce::FileChooser> openFileChooser;
    std::unique_ptr<juce::FileChooser> saveFileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
