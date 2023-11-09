/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================



SpecterAudioProcessorEditor::SpecterAudioProcessorEditor (SpecterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Initialize the ball position from the processor
    ballPosition = juce::Point<float>(audioProcessor.ballPosX.load() * getWidth(), 
                                      audioProcessor.ballPosY.load() * getHeight());
    // Adjust the size to add space for the toolbar
    int toolbarHeight = 50;
    setSize(600, 400 + toolbarHeight);

    // Set up the folderButton
    folderButton.setButtonText("Load");
    folderButton.addListener(this);
    addAndMakeVisible(folderButton);

    // Set up the diceButton
    diceButton.setButtonText("Dice");
    diceButton.addListener(this);
    diceButton.setEnabled(false); // Disable until a folder is selected
    addAndMakeVisible(diceButton);
    ballPosition = { getWidth() * 0.5f, getHeight() * 0.5f };
    
    // Set up the loopButton
    loopButton.setButtonText("Loop");
    loopButton.addListener(this);
    addAndMakeVisible(loopButton);
    loopButton.setToggleState(true, juce::dontSendNotification);
    
    isDragging = false;
    
    setMouseClickGrabsKeyboardFocus(false);
    setWantsKeyboardFocus(false);
    
    // Set up the rndMixButton
    rndMixButton.setButtonText("Rnd Mix");
    rndMixButton.addListener(this);
    rndMixButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(rndMixButton);
    startTimerHz(4);
    
    // Initialize the timerHzSlider
    timerHzSlider.setRange(30.0, 500.0, 0.1);
    timerHzSlider.setValue(200); // Starting value (you had startTimerHz(4) before)
    timerHzSlider.addListener(this);
    addAndMakeVisible(timerHzSlider);
    
    // Set up the stopButton
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);
    
    filterButton.setButtonText("/");
    filterButton.addListener(this);
    filterButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(filterButton);
    
    reverbButton.setButtonText(")");
    reverbButton.addListener(this);
    reverbButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(reverbButton);
    
    granularButton.setButtonText("**");
    granularButton.addListener(this);
    granularButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(granularButton);
    
    loopTempoButton.setButtonText("\\");
    loopTempoButton.addListener(this);
    loopTempoButton.setEnabled(true); // Enable or disable as per your needs
    addAndMakeVisible(loopTempoButton);
}

SpecterAudioProcessorEditor::~SpecterAudioProcessorEditor()
{
    // Your cleanup code here
}

void SpecterAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    
    if (button == &folderButton)
    {
        juce::FileChooser chooser("Select a folder containing .wav or .aif files", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "");

        if (chooser.browseForDirectory())
        {
            juce::File folder = chooser.getResult();

            audioFiles.clear(); // Clear the previous list

            // Fetch .wav files
            folder.findChildFiles(audioFiles, juce::File::findFiles, false, "*.wav");

            // Fetch .aif files
            folder.findChildFiles(audioFiles, juce::File::findFiles, false, "*.aif");

            // Log the results
            juce::String message = "Found " + juce::String(audioFiles.size()) + " audio files.";
            juce::Logger::writeToLog(message);
            
            if (audioFiles.size() > 0)
            {
                juce::Array<juce::File> filesToPlay;
                for (int i = 0; i < juce::jmin(4, audioFiles.size()); ++i)
                {
                    filesToPlay.add(audioFiles[i]);
                }

                // Get the processor and update it with the new files
                auto* processor = dynamic_cast<SpecterAudioProcessor*>(getAudioProcessor());
                if (processor != nullptr)  // Always check for a valid pointer after a dynamic_cast
                {
                    processor->loadFiles(filesToPlay);     // Load the files for playback
                    processor->updateAudioFiles(audioFiles); // Update the processor with the full list of files
                }
            }       

            diceButton.setEnabled(audioFiles.size() >= 4);
        }
    }
  else if (button == &diceButton)
{
    if (audioFiles.size() >= 4)
    {
        // Shuffle the audioFiles array to randomize
            juce::Random r;
            for (int i = audioFiles.size(); --i >= 1;)
            {
                auto swapIndex = r.nextInt(i + 1);
                audioFiles.swap(swapIndex, i);
            }

            // Now pick the first four files after shuffling
            juce::Array<juce::File> filesToPlay = { audioFiles[0], audioFiles[1], audioFiles[2], audioFiles[3] };

            // Pass the files to the processor
            auto* processor = dynamic_cast<SpecterAudioProcessor*>(getAudioProcessor());
            if (processor != nullptr)
            {
                processor->loadFiles(filesToPlay);
            }
            processor->updateAudioFiles(audioFiles);
            repaint(); // This will trigger a repaint to update any UI components
    }
    else
    {
        // If for some reason there are fewer than 4 audio files, you can add some error handling here.
        juce::Logger::writeToLog("Not enough audio files to shuffle and pick four.");
    }
}
    if (button == &loopButton)
    {
        // Cast getAudioProcessor() to your processor type
        auto* processor = dynamic_cast<SpecterAudioProcessor*>(getAudioProcessor());
        if (processor) // Always check if the cast was successful
        {
            processor->setLooping(loopButton.getToggleState());
        }
    }
     else if (button == &rndMixButton)
    {
        // Ensure no movement is happening while setting up new points
        shouldMoveBall = false;

        // Generate 4 random points for the joystick to move to
        for (int i = 0; i < 4; ++i)
        {
            points[i].x = juce::Random::getSystemRandom().nextFloat() * (getWidth() - 30) + 15; // 15 and 30 are ball radii and diameter
            points[i].y = juce::Random::getSystemRandom().nextFloat() * (getHeight() - 30 - 50) + 50 + 15; // 50 is the toolbar height
        }

        currentPointIndex = 0; // Reset the current point index

        // Set the initial ball position to the first point to avoid a visual jump
        ballPosition = points[0];
        
        int timerInterval = static_cast<int>(1000.0 / timerHzSlider.getValue());

        // Restart the timer here if it's not always running
        startTimer(timerInterval);

        // Now allow the ball to move
        shouldMoveBall = true;
    }
    else if (button == &stopButton)
    {
        shouldMoveBall = false; // This will stop the ball from moving
    }
    if (button == &reverbButton)
    {
        // Randomize the reverb parameters every time the button is clicked
        audioProcessor.randomizeReverbParameters();

        // Ensure the reverb is turned on
        audioProcessor.apvts.getParameterAsValue("reverbButton").setValue(true);
    }
    if (button == &filterButton)
    {
        // Randomize the reverb parameters every time the button is clicked
        audioProcessor.randomizeLowPassFilterParameters();

        // Ensure the reverb is turned on
        audioProcessor.apvts.getParameterAsValue("filterButton").setValue(true);
    }

}



//==============================================================================
void SpecterAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto* processor = dynamic_cast<SpecterAudioProcessor*>(getAudioProcessor());
    if (processor == nullptr) return; // Exit if the processor is not the expected type

    // Fetch the audio files from the processor
    const auto& audioFilesFromProcessor = processor->getAudioFiles();
    g.fillAll(juce::Colours::black);  
    
    // Draw the toolbar
    int toolbarHeight = 50;
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(0, 0, getWidth(), toolbarHeight);

    // Adjust quadrant lines
    g.setColour(juce::Colours::white);
    g.drawLine(getWidth() * 0.5, toolbarHeight, getWidth() * 0.5, getHeight(), 2.0f);

    int horizontalLineY = (getHeight() - toolbarHeight) * 0.5 + toolbarHeight;
    g.drawLine(0, horizontalLineY, getWidth(), horizontalLineY, 2.0f);

    // Adjust the blue ball
    g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(ballPosition.x - 15, ballPosition.y - 15, 30, 30);
    
    // Display the file names
    if (audioFilesFromProcessor.size() >= 4)
    {
        g.setColour(juce::Colours::white);
        g.drawText(audioFilesFromProcessor[0].getFileName(), 10, getHeight() * 0.25, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFilesFromProcessor[1].getFileName(), getWidth() * 0.5 + 10, getHeight() * 0.25, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFilesFromProcessor[2].getFileName(), 10, getHeight() * 0.75, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
        g.drawText(audioFilesFromProcessor[3].getFileName(), getWidth() * 0.5 + 10, getHeight() * 0.75, getWidth() * 0.5 - 20, 20, juce::Justification::centred);
    }
    
}


void SpecterAudioProcessorEditor::shuffleAudioFiles()
{
    juce::Random random; // Initialize a random number generator

    for (int i = audioFiles.size() - 1; i > 0; i--)
    {
        // Generate a random index between 0 and i
        int j = random.nextInt(i + 1);

        // Swap audioFiles[i] with the element at random index j
        audioFiles.swap(i, j);
    }
}
void SpecterAudioProcessorEditor::resized()
{
    int toolbarHeight = 50; // Height of the toolbar

    // Let's space out the buttons a little bit within the toolbar
    int buttonYPosition = 10; // This will center them vertically in the toolbar (given their height of 30)
    int buttonSpacing = 10;  // Horizontal spacing between buttons

    folderButton.setBounds(buttonSpacing, buttonYPosition, 60, 30);
    
    // Place the diceButton to the right of the folderButton, with the defined spacing
    diceButton.setBounds(folderButton.getRight() + buttonSpacing, buttonYPosition, 60, 30);
    rndMixButton.setBounds(diceButton.getRight() + buttonSpacing, buttonYPosition, 60, 30);
    stopButton.setBounds(rndMixButton.getRight() + buttonSpacing, buttonYPosition, 40, 20);
    timerHzSlider.setBounds(stopButton.getRight() + buttonSpacing, buttonYPosition, 80, 20);
    filterButton.setBounds(timerHzSlider.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    reverbButton.setBounds(filterButton.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    granularButton.setBounds(reverbButton.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    loopTempoButton.setBounds(granularButton.getRight() + buttonSpacing, buttonYPosition, 20, 20);
    loopButton.setBounds(loopTempoButton.getRight() + buttonSpacing, buttonYPosition, 60, 20);

}

//=====
//Joystick movement:

void SpecterAudioProcessorEditor::computeMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight)
{
    float x = ballPosition.x / getWidth();
    float y = ballPosition.y / getHeight();

    topLeft = (1 - x) * (1 - y);
    topRight = x * (1 - y);
    bottomLeft = (1 - x) * y;
    bottomRight = x * y;
    
     if (auto* processor = dynamic_cast<SpecterAudioProcessor*>(getAudioProcessor()))
    {
        processor->setMixLevels(topLeft, topRight, bottomLeft, bottomRight);
    }
}


void SpecterAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    // Check if the mouse down position is within the ball's region
    float distance = ballPosition.getDistanceFrom(e.position);
    if (distance <= 15.0f) // 15.0f is the ball's radius
    {
        isDragging = true;
    }
}

void SpecterAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
}

void SpecterAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
        
    {
        ballPosition = event.position;
    // Boundary checks
    if (ballPosition.x < 15) ballPosition.x = 15;
    if (ballPosition.x > getWidth() - 15) ballPosition.x = getWidth() - 15;
    if (ballPosition.y < 15 + 50) ballPosition.y = 15 + 50;  // 50 is the toolbar height
    if (ballPosition.y > getHeight() - 15) ballPosition.y = getHeight() - 15;
        
        float topLeft, topRight, bottomLeft, bottomRight;
        computeMixLevels(topLeft, topRight, bottomLeft, bottomRight);
        audioProcessor.setMixLevels(topLeft, topRight, bottomLeft, bottomRight);
        audioProcessor.ballPosX.store(ballPosition.x / getWidth());
        audioProcessor.ballPosY.store(ballPosition.y / getHeight());
    DBG("Mix Levels: " + juce::String(topLeft) + ", " + juce::String(topRight) + ", " + juce::String(bottomLeft) + ", " + juce::String(bottomRight));
    repaint();

        repaint();  // Redraw with new ball position
    }
}

void SpecterAudioProcessorEditor::timerCallback()
{
    if (shouldMoveBall) {
        // Calculate the vector from the current ball position to the target point
        juce::Point<float> direction = points[currentPointIndex] - ballPosition;

        // Check if we are close enough to the target point to snap to it
        const float snapThreshold = 0.5f;
        if (direction.getDistanceFromOrigin() < snapThreshold) {
            ballPosition = points[currentPointIndex]; // Snap to target point
            

            // Increment the current point index and wrap it around if it exceeds the number of points
            currentPointIndex = (currentPointIndex + 1) % 4;
            
        } else {
            // Normalize the direction vector to a unit vector
            direction = direction / direction.getDistanceFromOrigin();

            // Move the ball by a small step towards the target point
            const float stepSize = 1.0f;
            ballPosition += direction * stepSize;
        
        // Once the ball reaches the point, update the processor with the new position
        audioProcessor.ballPosX.store(ballPosition.x / getWidth());
        audioProcessor.ballPosY.store((ballPosition.y - 50) / (getHeight() - 50));

        }

        // Update the mix levels based on new ball position
        float topLeft, topRight, bottomLeft, bottomRight;
        computeMixLevels(topLeft, topRight, bottomLeft, bottomRight);
        audioProcessor.setMixLevels(topLeft, topRight, bottomLeft, bottomRight);

        // Repaint to show the ball's new position.
        repaint();
    }
    
    
}

void SpecterAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &timerHzSlider)
    {
        startTimerHz(timerHzSlider.getValue());
    }
}

