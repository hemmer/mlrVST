/*
  ==============================================================================

    Preset.cpp
    Created: 3 Nov 2013 12:09:29pm
    Author:  hemmer

  ==============================================================================
*/

#include "Preset.h"
#include "SampleStrip.h"
#include "GlobalSettings.h"


XmlElement Preset::createPreset(const String &presetName,
                                mlrVSTAudioProcessor * processor,
                                GlobalSettings *gs)
{
      // TODO: check name not blank
    XmlElement newPreset(presetName);

    // this is now the current preset

    // TODO: this needs sorted!!!!
    //setGlobalSetting(sPresetName, &newPresetName);


    for (int s = 0; s < GlobalSettings::NumGlobalSettings; ++s)
    {
        // skip if we don't save this setting with each preset
        if (GlobalSettings::getSettingPresetScope(s) != GlobalSettings::ScopePreset)
            continue;

        // find if the setting is a bool, int, double etc.
        const int settingType = GlobalSettings::getGlobalSettingType(s);
        // what description do we write into the xml for this setting
        const String settingName = GlobalSettings::getGlobalSettingName(s);
        // get the actual parameter value (we cast later)
        const void *p = gs->getGlobalSetting(s);


        // some settings are in arrays (e.g. channel gains): these
        // need special treatment
        if (settingType | GlobalSettings::TypeArray)
        {
            const int arraySettingType = settingType - GlobalSettings::TypeArray;
            const int numItems = gs->getGlobalSettingArrayLength(settingType);

            XmlElement *arrayItems = new XmlElement(settingName);

            switch (arraySettingType)
            {

            case GlobalSettings::TypeBool :
                {
                    for (int i = 0; i < numItems; ++i)
                    {
                        const bool boolSetting = *static_cast<const bool*>(gs->getGlobalSettingArray(settingType, i));
                        arrayItems->setAttribute(settingName+String(i), boolSetting);
                    }
                }

            case GlobalSettings::TypeFloat :
                {
                    for (int i = 0; i < numItems; ++i)
                    {
                        const float floatSetting = *static_cast<const float*>(gs->getGlobalSettingArray(settingType, i));
                        arrayItems->setAttribute(settingName+String(i), floatSetting);
                    }
                }

            default :
                DBG("Array setting type not found"); jassertfalse;
            }
        }
        else
        {
            switch (settingType)
            {
            case GlobalSettings::TypeBool :
                newPreset.setAttribute(settingName, (int)(*static_cast<const bool*>(p))); break;
            case GlobalSettings::TypeInt :
                newPreset.setAttribute(settingName, *static_cast<const int*>(p)); break;
            case GlobalSettings::TypeDouble :
                newPreset.setAttribute(settingName, *static_cast<const double*>(p)); break;
            case GlobalSettings::TypeFloat :
                newPreset.setAttribute(settingName, (double)(*static_cast<const float*>(p))); break;
            case GlobalSettings::TypeString :
                newPreset.setAttribute(settingName, (*static_cast<const String*>(p))); break;

            default : jassertfalse;
            }
        }

    }





    // Store the SampleStrip specific settings
    for (int strip = 0; strip < gs->numSampleStrips; ++strip)
    {
        XmlElement *stripXml = new XmlElement("STRIP");
        stripXml->setAttribute("id", strip);

        // write all parameters to XML
        for (int param = 0; param < SampleStrip::TotalNumParams; ++param)
        {
            // check if we are supposed to save this parameter, if not, skip
            if (!SampleStrip::isParamSaved(param)) continue;

            // find if the parameter is a bool, int, double etc.
            const int paramType = SampleStrip::getParameterType(param);
            // what description do we write into the xml for this parameter
            const String paramName = SampleStrip::getParameterName(param);
            // get the actual parameter value (we cast later)
            const void *p = processor->getSampleStripParameter(param, strip);

            switch (paramType)
            {
            case SampleStrip::TypeBool :
                stripXml->setAttribute(paramName, (int)(*static_cast<const bool*>(p))); break;
            case SampleStrip::TypeInt :
                stripXml->setAttribute(paramName, *static_cast<const int*>(p)); break;
            case SampleStrip::TypeDouble :
                stripXml->setAttribute(paramName, *static_cast<const double*>(p)); break;
            case SampleStrip::TypeFloat :
                stripXml->setAttribute(paramName, (double)(*static_cast<const float*>(p))); break;
            case SampleStrip::TypeAudioSample :
                {
                    const AudioSample * sample = static_cast<const AudioSample*>(p);
                    if (sample)
                    {
                        const String samplePath = sample->getSampleFile().getFullPathName();
                        stripXml->setAttribute(paramName, samplePath);
                    }
                    break;
                }
            default : jassertfalse;
            }
        }

        // add this strip to the preset
        newPreset.addChildElement(stripXml);


    } // end loop over sample strips

    return newPreset;

    //const String nameAttribute = GlobalSettings::getGlobalSettingName(GlobalSettings::sPresetName);
    //bool duplicateFound = false;

    //// See if a preset of this name exists in the preset list
    //forEachXmlChildElement(presetList, p)
    //{
    //    const String pName = p->getStringAttribute(nameAttribute);

    //    // If it does, replace it
    //    if (pName == newPresetName)
    //    {
    //        presetList.replaceChildElement(p, new XmlElement(newPreset));
    //        duplicateFound = true;
    //        DBG("Replacing preset: '" << pName << "'.");
    //        break;
    //    }
    //}

    //// If the name is unique, add the new preset
    //if (!duplicateFound) presetList.addChildElement(new XmlElement(newPreset));

    //DBG(presetList.createDocument(String::empty));

}


void Preset::loadPreset(XmlElement * presetToLoad,
                        mlrVSTAudioProcessor * processor,
                        GlobalSettings *gs)
{
        // this *should* exist, if not, return doing nothing
    //if (presetToLoad == nullptr)
    //{
    //    jassertfalse;
    //    return;
    //}


    //if (presetToLoad->getTagName() == "PRESETNONE")
    //{
    //    // we have a blank preset: loadDefaultPreset();
    //    DBG("blank preset loaded");
    //}
    //else if (presetToLoad->getTagName() == "PRESET")
    //{
    //    const String newPresetName = presetToLoad->getStringAttribute("preset_name", "NOT FOUND");
    //    // check we know the name
    //    DBG("preset \"" << newPresetName << "\" loaded.");

    //    // TODO: first load the global parameters

    //    for (int s = 0; s < GlobalSettings::NumGlobalSettings; ++s)
    //    {
    //        // skip if we don't save this setting with each preset
    //        if (GlobalSettings::getSettingPresetScope(s) != GlobalSettings::ScopePreset)
    //            continue;

    //        // find if the setting is a bool, int, double etc.
    //        const int settingType = GlobalSettings::getGlobalSettingType(s);
    //        // what description do we write into the xml for this setting
    //        const String settingName = GlobalSettings::getGlobalSettingName(s);

    //        switch (settingType)
    //        {
    //        case SampleStrip::TypeBool :
    //            {
    //                const bool value = presetToLoad->getIntAttribute(settingName) != 0;
    //                setGlobalSetting(s, &value, false); break;
    //            }
    //        case SampleStrip::TypeInt :
    //            {
    //                const int value = presetToLoad->getIntAttribute(settingName);
    //                setGlobalSetting(s, &value, false); break;
    //            }
    //        case SampleStrip::TypeDouble :
    //            {
    //                const double value = presetToLoad->getDoubleAttribute(settingName);
    //                setGlobalSetting(s, &value, false); break;
    //            }
    //        case SampleStrip::TypeFloat :
    //            {
    //                const float value = (float) presetToLoad->getDoubleAttribute(settingName);
    //                setGlobalSetting(s, &value, false); break;
    //            }
    //        case SampleStrip::TypeString :
    //            {
    //                const String value = presetToLoad->getStringAttribute(settingName);
    //                setGlobalSetting(s, &value, false); break;
    //            }
    //        default : jassertfalse;
    //        }

    //    }


    //    const float newMasterGain = (float) presetToLoad->getDoubleAttribute("master_vol", defaultChannelGain);
    //    if (newMasterGain >= 0.0 && newMasterGain <= 1.0) gs.masterGain = newMasterGain;

    //    for (int c = 0; c < gs.numChannels; ++c)
    //    {
    //        const String chanVolName = "chan_" + String(c) + "_vol";
    //        const float chanGain = (float) presetToLoad->getDoubleAttribute(chanVolName, defaultChannelGain);
    //        channelGains.set(c, (chanGain >= 0.0 && chanGain <= 1.0) ? chanGain : defaultChannelGain);
    //    }



    //    // try to find settings for each strip *that currently exists*
    //    for (int s = 0; s < sampleStripArray.size(); ++s)
    //    {
    //        // search the preset for the strip with matching ID
    //        forEachXmlChildElement(*presetToLoad, strip)
    //        {
    //            const int stripID = strip->getIntAttribute("id", -1);
    //            // check if the preset has information for this SampleStrip
    //            if (stripID != s) continue;

    //            SampleStrip *currentStrip = sampleStripArray[stripID];

    //            // if so, try to extract all the required parameters
    //            for (int param = 0; param < SampleStrip::TotalNumParams; ++param)
    //            {
    //                // check that this is a parameter that we load
    //                const bool doWeLoadParam = SampleStrip::isParamSaved(param);
    //                if (!doWeLoadParam) continue;

    //                // what name is this parameter?
    //                const String paramName = SampleStrip::getParameterName(param);

    //                // check that it can be found in the XML
    //                if (strip->hasAttribute(paramName))
    //                {
    //                    // find its type
    //                    const int paramType = SampleStrip::getParameterType(param);

    //                    // DBG("id: " << stripID << " " << paramName << " " << param << " " << paramType);

    //                    // try to load it
    //                    switch (paramType)
    //                    {
    //                    case TypeInt :
    //                        {
    //                            // TODO: validate values here
    //                            const int value = strip->getIntAttribute(paramName);
    //                            currentStrip->setSampleStripParam(param, &value);
    //                            break;
    //                        }
    //                    case TypeFloat :
    //                        {
    //                            const float value = (float) strip->getDoubleAttribute(paramName);
    //                            currentStrip->setSampleStripParam(param, &value);
    //                            break;
    //                        }
    //                    case TypeDouble :
    //                        {
    //                            const double value = strip->getDoubleAttribute(paramName);
    //                            currentStrip->setSampleStripParam(param, &value);
    //                            break;
    //                        }
    //                    case TypeBool :
    //                        {
    //                            const bool value = strip->getBoolAttribute(paramName);
    //                            currentStrip->setSampleStripParam(param, &value);
    //                            break;
    //                        }
    //                    case TypeAudioSample :
    //                        {
    //                            const String filePath = strip->getStringAttribute(paramName);
    //                            File newFile = File(filePath);

    //                            // add the sample and get the id
    //                            const int sampleID = addNewSample(newFile);
    //                            const AudioSample * newSample = getAudioSample(sampleID, pSamplePool);

    //                            // if the sample loaded correctly, set it to be the strip's AudioSample
    //                            if (sampleID != -1 && newSample)
    //                                currentStrip->setSampleStripParam(SampleStrip::pAudioSample, newSample);
    //                        }
    //                    } // end switch statement

    //                } // end of if(hasAtttribute)
    //                else
    //                {
    //                    DBG("Param " << paramName << " not found. Doing nothing");
    //                    // TODO: load default
    //                    // OR do nothing
    //                }

    //            } // end loop over expected parameters

    //            // we have loaded this strip's parameters so can stop searching
    //            break;

    //        } // end of loop over preset entries

    //    }// end of samplestrip loop

    //    // let the GUI know that we have reloaded
    //    processor->sendChangeMessage();
    //}
}