#include "MainComponent.h"
#include "license.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace
{
    juce::Colour defaultStatusColour() { return juce::Colours::lightgrey; }
    juce::Colour validColour() { return juce::Colours::green; }
    juce::Colour invalidColour() { return juce::Colours::red; }
    juce::Colour errorColour() { return juce::Colours::orange; }
}

MainComponent::MainComponent()
{
    setupEditors();
    setupButtons();

    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, defaultStatusColour());
    statusLabel.setText("Ready", juce::dontSendNotification);

    setSize (720, 420);
}

MainComponent::~MainComponent() = default;

void MainComponent::setupEditors()
{
    auto configure = [this](juce::TextEditor& editor, const juce::String& placeholder)
    {
        addAndMakeVisible(editor);
        editor.setMultiLine(false);
        editor.setJustification(juce::Justification::centredLeft);
        editor.setTextToShowWhenEmpty(placeholder, juce::Colours::darkgrey);
    };

    configure(firstEdit, "First name");
    configure(lastEdit, "Last name");
    configure(emailEdit, "name@example.com");

    addAndMakeVisible(keyOut);
    keyOut.setReadOnly(true);
    keyOut.setMultiLine(false);
    keyOut.setTextToShowWhenEmpty("License key will appear here", juce::Colours::darkgrey);
    keyOut.onTextChange = [this]() { updateCopyState(); };
}

void MainComponent::setupButtons()
{
    auto configure = [this](juce::TextButton& btn, auto&& handler)
    {
        addAndMakeVisible(btn);
        btn.onClick = handler;
    };

    configure(btnGenerate, [this]() { generateLicense(); });
    configure(btnVerify, [this]() { verifyCurrentLicense(); });
    configure(btnCopy, [this]() { copyLicenseToClipboard(); });
    configure(btnBatchIn, [this]() { loadBatchFromCsv(); });
    configure(btnSaveCsv, [this]() { saveBatchToCsv(); });

    btnCopy.setEnabled(false);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    auto topRow = area.removeFromTop(44);
    juce::FlexBox topFlex;
    topFlex.flexDirection = juce::FlexBox::Direction::row;
    topFlex.items.add(juce::FlexItem(firstEdit).withFlex(1.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    topFlex.items.add(juce::FlexItem(lastEdit).withFlex(1.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    topFlex.items.add(juce::FlexItem(emailEdit).withFlex(1.4f));
    topFlex.performLayout(topRow);

    area.removeFromTop(12);
    auto buttonRow = area.removeFromTop(44);
    juce::FlexBox buttonFlex;
    buttonFlex.flexDirection = juce::FlexBox::Direction::row;
    buttonFlex.items.add(juce::FlexItem(btnGenerate).withFlex(1.0f).withMinWidth(100.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    buttonFlex.items.add(juce::FlexItem(btnVerify).withFlex(1.0f).withMinWidth(100.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    buttonFlex.items.add(juce::FlexItem(btnCopy).withFlex(1.0f).withMinWidth(100.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    buttonFlex.items.add(juce::FlexItem(btnBatchIn).withFlex(1.2f).withMinWidth(140.0f).withMargin(juce::FlexItem::Margin(0, 8, 0, 0)));
    buttonFlex.items.add(juce::FlexItem(btnSaveCsv).withFlex(1.2f).withMinWidth(120.0f));
    buttonFlex.performLayout(buttonRow);

    area.removeFromTop(12);
    auto keyArea = area.removeFromTop(48);
    keyOut.setBounds(keyArea);

    area.removeFromTop(12);
    auto statusArea = area.removeFromTop(24);
    statusLabel.setBounds(statusArea);
}

void MainComponent::updateStatus(const juce::String& message, juce::Colour colour)
{
    statusLabel.setColour(juce::Label::textColourId, colour);
    statusLabel.setText(message, juce::dontSendNotification);
}

bool MainComponent::validateInputs(juce::String& outFirst, juce::String& outLast, juce::String& outEmail)
{
    outFirst = firstEdit.getText().trim();
    outLast = lastEdit.getText().trim();
    outEmail = emailEdit.getText().trim();

    if (outFirst.isEmpty() || outLast.isEmpty() || outEmail.isEmpty())
    {
        updateStatus("Please enter first, last, and email.", errorColour());
        return false;
    }

    return true;
}

void MainComponent::updateCopyState()
{
    btnCopy.setEnabled(! keyOut.getText().isEmpty());
}

void MainComponent::generateLicense()
{
    juce::String first, last, email;
    if (!validateInputs(first, last, email))
        return;

    const std::string licenseKey = license::makeLicense(first.toStdString(),
                                                        last.toStdString(),
                                                        email.toStdString());
    keyOut.setText(licenseKey, juce::dontSendNotification);
    keyOut.selectAll();
    updateCopyState();
    updateStatus("Generated.", defaultStatusColour());
}

void MainComponent::verifyCurrentLicense()
{
    juce::String first, last, email;
    if (!validateInputs(first, last, email))
        return;

    const auto licenseText = keyOut.getText().trim();
    if (licenseText.isEmpty())
    {
        updateStatus("No license to verify.", errorColour());
        return;
    }

    const bool valid = license::verifyLicense(licenseText.toStdString(),
                                              first.toStdString(),
                                              last.toStdString(),
                                              email.toStdString());
    updateStatus(valid ? "Valid" : "Invalid", valid ? validColour() : invalidColour());
}

void MainComponent::copyLicenseToClipboard()
{
    const auto text = keyOut.getText();
    if (text.isEmpty())
        return;

    juce::SystemClipboard::copyTextToClipboard(text);
    updateStatus("Copied to clipboard.", defaultStatusColour());
}

void MainComponent::loadBatchFromCsv()
{
    openFileChooser = std::make_unique<juce::FileChooser>("Select CSV file to open", juce::File{}, "*.csv");
    if (auto* chooser = openFileChooser.get())
    {
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser& fc)
                             {
                                 auto file = fc.getResult();
                                 if (file.existsAsFile())
                                 {
                                     juce::FileInputStream stream(file);
                                     if (! stream.openedOk())
                                     {
                                         updateStatus("Failed to open file.", errorColour());
                                         openFileChooser.reset();
                                         return;
                                     }

                                     const juce::String content = stream.readEntireStreamAsString();
                                     juce::StringArray lines;
                                     lines.addLines(content);

                                     if (lines.isEmpty())
                                     {
                                         updateStatus("CSV is empty.", errorColour());
                                         openFileChooser.reset();
                                         return;
                                     }

                                     auto looksLikeHeader = [](const juce::StringArray& columns)
                                     {
                                         if (columns.size() < 3)
                                             return false;
                                         return columns[0].trim().equalsIgnoreCase("first") &&
                                                columns[1].trim().equalsIgnoreCase("last") &&
                                                columns[2].trim().equalsIgnoreCase("email");
                                     };

                                     batchRows.clear();
                                     batchRows.reserve(static_cast<size_t>(lines.size()));

                                     for (int i = 0; i < lines.size(); ++i)
                                     {
                                         auto columns = juce::StringArray::fromTokens(lines[i], ",", "");
                                         for (int c = 0; c < columns.size(); ++c)
                                             columns.set(c, columns[c].trim());

                                         if (i == 0 && looksLikeHeader(columns))
                                             continue;

                                         if (columns.size() < 3)
                                             continue;

                                         if (columns[0].isEmpty() || columns[1].isEmpty() || columns[2].isEmpty())
                                             continue;

                                         Row row;
                                         row.first = columns[0];
                                         row.last = columns[1];
                                         row.email = columns[2];
                                         row.license = license::makeLicense(row.first.toStdString(),
                                                                            row.last.toStdString(),
                                                                            row.email.toStdString());
                                         batchRows.push_back(row);
                                     }

                                     if (batchRows.empty())
                                     {
                                         updateStatus("No rows parsed.", errorColour());
                                     }
                                     else
                                     {
                                         updateStatus(juce::String(batchRows.size()) + " licenses generated.", defaultStatusColour());
                                     }
                                 }

                                 openFileChooser.reset();
                             });
    }
}

void MainComponent::saveBatchToCsv()
{
    if (batchRows.empty())
    {
        updateStatus("No batch data to save.", errorColour());
        return;
    }

    saveFileChooser = std::make_unique<juce::FileChooser>("Select CSV file to save", juce::File{}, "*.csv");
    if (auto* chooser = saveFileChooser.get())
    {
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser& fc)
                             {
                                 auto file = fc.getResult();
                                 if (file != juce::File{})
                                 {
                                     juce::FileOutputStream stream(file);
                                     if (! stream.openedOk())
                                     {
                                         updateStatus("Failed to save file.", errorColour());
                                         saveFileChooser.reset();
                                         return;
                                     }

                                     stream << "first,last,email,license\n";
                                     for (const auto& row : batchRows)
                                     {
                                         stream << row.first << ','
                                                << row.last << ','
                                                << row.email << ','
                                                << row.license << '\n';
                                     }

                                     stream.flush();
                                     updateStatus("CSV saved.", defaultStatusColour());
                                 }

                                 saveFileChooser.reset();
                             });
    }
}
