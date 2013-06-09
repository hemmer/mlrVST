/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_mlrVSTLookAndFeel_JUCEHEADER__
#define __JUCE_mlrVSTLookAndFeel_JUCEHEADER__

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fonts/FreeTypeFaces.h"





//==============================================================================
/**
    The original Juce look-and-feel.

*/
class mlrVSTLookAndFeel : public LookAndFeel
{
public:
    //==============================================================================
    /** Creates the default JUCE look and feel. */
    mlrVSTLookAndFeel();

    /** Destructor. */
    virtual ~mlrVSTLookAndFeel();

    //==============================================================================
    /** Draws the background for a standard button. */
    virtual void drawButtonBackground (Graphics& g, Button& button,
                                       const Colour& backgroundColour,
                                       bool isMouseOverButton,
                                       bool isButtonDown);

    virtual void drawButtonText (Graphics& g, TextButton& button,
                                 bool isMouseOverButton, bool isButtonDown);

    /** Draws the contents of a standard ToggleButton. */
    virtual void drawToggleButton (Graphics& g, ToggleButton& button,
                                   bool isMouseOverButton,
                                   bool isButtonDown);

    virtual void drawLabel(Graphics& g, Label& label);
    virtual const Typeface::Ptr getTypefaceForFont(const Font &font );
	virtual Font getTextButtonFont(TextButton & button);



    virtual void drawTickBox (Graphics& g, Component& component,
                              float x, float y, float w, float h,
                              bool ticked, bool isEnabled,
                              bool isMouseOverButton, bool isButtonDown);

    //==============================================================================
    virtual void drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                  int width, int height,
                                  double progress, const String& textToShow);

    //==============================================================================
    virtual void drawScrollbarButton (Graphics& g, ScrollBar& scrollbar,
                                      int width, int height, int buttonDirection,
                                      bool isScrollbarVertical, bool isMouseOverButton,
                                      bool isButtonDown);

    virtual void drawScrollbar (Graphics& g, ScrollBar& scrollbar,
                                int x, int y, int width, int height,
                                bool isScrollbarVertical, int thumbStartPosition,
                                int thumbSize, bool isMouseOver, bool isMouseDown);

    virtual ImageEffectFilter* getScrollbarEffect();


    //==============================================================================
    /** Fills the background of a popup menu component. */
    virtual void drawPopupMenuBackground (Graphics& g, int width, int height);

    virtual void drawMenuBarBackground (Graphics& g, int width, int height,
                                        bool isMouseOverBar, MenuBarComponent& menuBar);

    //==============================================================================
    virtual void drawComboBox (Graphics& g, int width, int height,
                               bool isButtonDown,
                               int buttonX, int buttonY,
                               int buttonW, int buttonH,
                               ComboBox& box);

    virtual Font getComboBoxFont(ComboBox& box);
    virtual Font getPopupMenuFont();

    //==============================================================================
    virtual void drawLinearSlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider);

    virtual int getSliderThumbRadius (Slider& slider);

    virtual Button* createSliderButton (bool isIncrement);

    virtual ImageEffectFilter* getSliderEffect();

    //==============================================================================
    virtual void drawCornerResizer (Graphics& g,
                                    int w, int h,
                                    bool isMouseOver,
                                    bool isMouseDragging);

    virtual Button* createDocumentWindowButton (int buttonType);

    virtual void positionDocumentWindowButtons (DocumentWindow& window,
                                                int titleBarX, int titleBarY,
                                                int titleBarW, int titleBarH,
                                                Button* minimiseButton,
                                                Button* maximiseButton,
                                                Button* closeButton,
                                                bool positionTitleBarButtonsOnLeft);

private:
    //==============================================================================
    DropShadowEffect scrollbarShadow;
	const Font defaultFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (mlrVSTLookAndFeel);

    Typeface::Ptr typeSilk;


};



class overrideLookandFeel : public mlrVSTLookAndFeel
{
	// this LookAndFeel subclasses the main mlrVSTLookAndFeel
	// class to override the drawLabel method. This is because 
	// some components can't manually set the font they want to 
	// use.
public:

	overrideLookandFeel(){
		FreeTypeFaces::addFaceFromMemory(7.f, 25.f, true, BinaryData::VERDANA_TTF, BinaryData::VERDANA_TTFSize);
	}
	
	const Typeface::Ptr getTypefaceForFont (Font const& font)
	{
		Typeface::Ptr tf;

		// get the hinted typeface.
		tf = FreeTypeFaces::createTypefaceForFont (font);

		// If we got here without creating a new typeface
		// then just use the default LookAndFeel behavior.
		if (!tf) tf = LookAndFeel::getTypefaceForFont (font);
		return tf;
	}

	void drawLabel(Graphics& g, Label& label)
    {
		// Here we override the with a specific font - this
		// is for classes which don't have a specific setFont
		// method.
		g.setFont(Font("Verdana", 10.f, Font::plain));

        g.fillAll (label.findColour(Label::backgroundColourId));


        const float alpha = label.isEnabled() ? 1.0f : 0.5f;

        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
		        

        g.drawFittedText(label.getText(), 4, 4,
            label.getWidth() - 2, label.getHeight() - 8,
            Justification::centredLeft, 1, 1.0f);

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
        g.drawRect (0, 0, label.getWidth(), label.getHeight());

    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (overrideLookandFeel);

};



#endif   // __JUCE_mlrVSTLookAndFeel_JUCEHEADER__
