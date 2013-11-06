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
    XmlElement newPreset("preset");

    // this is now the current preset

    // TODO: this needs sorted!!!!
    gs->setGlobalSetting(GlobalSettings::sPresetName, &presetName);


    //////////////////////////////////////////////////////////
    // Step 1. Write all global settings that are required. We
    // can get the scope of any given setting by calling:
    // GlobalSettings::getSettingPresetScope(settingID)

    for (int s = 0; s < GlobalSettings::NumGlobalSettings; ++s)
    {
        // skip if we don't save this setting with each preset
        if (GlobalSettings::getSettingPresetScope(s) != GlobalSettings::ScopePreset)
            continue;

        // find if the setting is a bool, int, double etc.
        const int settingType = GlobalSettings::getGlobalSettingType(s);
        // what description do we write into the xml for this setting
        const String settingName = GlobalSettings::getGlobalSettingName(s);


        const bool isThisSettingAnArray = (settingType & GlobalSettings::TypeArray) != 0;

        // some settings are in arrays (e.g. channel gains): these
        // need special treatment
        if (isThisSettingAnArray)
        {
            const int arraySettingType = settingType - GlobalSettings::TypeArray;
            const int numItems = gs->getGlobalSettingArrayLength(s);

            XmlElement *arrayItems = new XmlElement(settingName);

            switch (arraySettingType)
            {

            case GlobalSettings::TypeBool :
                {
                    for (int i = 0; i < numItems; ++i)
                    {
                        const bool boolSetting = *static_cast<const bool*>(gs->getGlobalSettingArray(s, i));
                        arrayItems->setAttribute(settingName+String(i), boolSetting);
                    }
                    break;
                }

            case GlobalSettings::TypeFloat :
                {
                    for (int i = 0; i < numItems; ++i)
                    {
                        const float floatSetting = *static_cast<const float*>(gs->getGlobalSettingArray(s, i));
                        arrayItems->setAttribute(settingName+String(i), floatSetting);
                    }
                    break;
                }

            default :
                DBG("Array setting type not found"); jassertfalse;
            }

            newPreset.addChildElement(arrayItems);

        }
        else
        {
            // If we are dealing with single values then get the actual
            // parameter value (we cast below based on settingType)
            const void *p = gs->getGlobalSetting(s);

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




    ///////////////////////////////////////////////////////////
    // Step 2. Write the SampleStrip specific settings for each
    // strip. We use SampleStrip::isParamSaved(param) to check
    // if that parameter is saved

    for (int strip = 0; strip < gs->numSampleStrips; ++strip)
    {
        XmlElement *stripXml = new XmlElement("strip");

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
                        // if we have a valid sample, store its filepath
                        const String samplePath = sample->getSampleFile().getFullPathName();
                        stripXml->setAttribute(paramName, samplePath);
                    }
                    break;
                }
            default : jassertfalse;
            }
        }

        // add this SampleStrip's parameters to the preset
        newPreset.addChildElement(stripXml);

    } // end loop over sample strips


    return newPreset;

}


bool Preset::loadPreset(XmlElement * presetToLoad,
                        mlrVSTAudioProcessor * processor,
                        GlobalSettings *gs)
{
    // this *should* exist, if not, return doing nothing
    if (presetToLoad == nullptr)
    {
        jassertfalse;
        return false;
    }

    DBG(presetToLoad->createDocument(String::empty));

    if (presetToLoad->getTagName() == "preset_none")
    {
        // we have a blank preset: loadDefaultPreset();
        DBG("blank preset loaded");
    }
    else if (presetToLoad->getTagName() == "preset")
    {

        const int numAttribs = presetToLoad->getNumAttributes();
        for (int i = 0; i < numAttribs; ++i)
        {
            const String attribName = presetToLoad->getAttributeName(i);
            const int globalSettingID = GlobalSettings::getGlobalSettingID(attribName);
            const int globalSettingType = GlobalSettings::getGlobalSettingType(globalSettingID);

            const String valueStr = presetToLoad->getAttributeValue(i);
            DBG("Loading attrib " << attribName << " with ID " << globalSettingID << " val " << valueStr);

            switch (globalSettingType)
            {
            case GlobalSettings::TypeBool :
                {
                    const bool value = valueStr.getIntValue() != 0;
                    gs->setGlobalSetting(globalSettingID, &value, false); break;
                }
            case GlobalSettings::TypeInt :
                {
                    const int value = valueStr.getIntValue();
                    gs->setGlobalSetting(globalSettingID, &value, false); break;
                }
            case GlobalSettings::TypeDouble :
                {
                    const double value = valueStr.getDoubleValue();
                    gs->setGlobalSetting(globalSettingID, &value, false); break;
                }
            case GlobalSettings::TypeFloat :
                {
                    const float value = (float) valueStr.getDoubleValue();
                    gs->setGlobalSetting(globalSettingID, &value, false); break;
                }
            case GlobalSettings::TypeString :
                {
                    gs->setGlobalSetting(globalSettingID, &valueStr, false); break;
                }
            default : jassertfalse;
            }
        }


        forEachXmlChildElement(*presetToLoad, elem)
        {
            const String arraySettingName = elem->getTagName();
            DBG(arraySettingName);
            DBG(elem->createDocument(String::empty));

            // if we are loading a SampleStrip's parameters
            if (arraySettingName == "strip")
            {
                // find which sample strip we are dealing with
                const int stripID = elem->getIntAttribute("id", -1);
                // and check it is valid
                if (stripID > gs->numSampleStrips || stripID < 0) continue;

                const int numAttribs = elem->getNumAttributes();
                for (int i = 0; i < numAttribs; ++i)
                {
                    const String sampleStripParamName = elem->getAttributeName(i);
                    const int sampleStripParamID = SampleStrip::getParameterID(sampleStripParamName);
                    const int sampleStripParamType = SampleStrip::getParameterType(sampleStripParamID);
                    // get the value as a string: use the type (found above) to
                    // convert it to the correct type
                    const String sampleStripParamVal = elem->getAttributeValue(i);


                    // TODO: validate values here (possibly create outer function
                    // setSampleStripParameterChecked to sanitize input)
                    switch (sampleStripParamType)
                    {
                    case SampleStrip::TypeInt :
                        {

                            const int value = sampleStripParamVal.getIntValue();
                            processor->setSampleStripParameter(sampleStripParamID, &value, stripID, false);
                            break;
                        }
                    case SampleStrip::TypeFloat :
                        {
                            const float value = sampleStripParamVal.getFloatValue();
                            processor->setSampleStripParameter(sampleStripParamID, &value, stripID, false);
                            break;
                        }
                    case SampleStrip::TypeDouble :
                        {
                            const float value = sampleStripParamVal.getDoubleValue();
                            processor->setSampleStripParameter(sampleStripParamID, &value, stripID, false);
                            break;
                        }
                    case SampleStrip::TypeBool :
                        {
                            const bool value = sampleStripParamVal.getIntValue() != 0;
                            processor->setSampleStripParameter(sampleStripParamID, &value, stripID, false);
                            break;
                        }
                    case SampleStrip::TypeAudioSample :
                        {
                            const String filePath = sampleStripParamVal;
                            File newFile = File(filePath);

                            // add the sample and get the id
                            const int sampleID = processor->addNewSample(newFile);

                            // continue if loading failed
                            if (sampleID == 0) continue;

                            const AudioSample * newSample = processor->getAudioSample(sampleID, mlrVSTAudioProcessor::pSamplePool);

                            // if the sample loaded correctly, set it to be the strip's AudioSample
                            if (newSample)
                                processor->setSampleStripParameter(SampleStrip::pAudioSample, newSample, stripID, false);
                        }
                    } // end switch statement

                }

                // now that all settings have been updated, fire
                // off a change message so that the GUI can update
                processor->sendSampleStripChangeMsg(stripID);
            }
            else if (arraySettingName == "channel_gains")
            {
                const int numAttribs = elem->getNumAttributes();
                for (int c = 0; c < numAttribs; ++c)
                {
                    // TODO !
                }
            }

        }


        //// try to find settings for each strip *that currently exists*
        //for (int s = 0; s < sampleStripArray.size(); ++s)
        //{
        //    // search the preset for the strip with matching ID
        //    forEachXmlChildElement(*presetToLoad, strip)
        //    {
        //        const int stripID = strip->getIntAttribute("id", -1);
        //        // check if the preset has information for this SampleStrip
        //        if (stripID != s) continue;

        //        SampleStrip *currentStrip = sampleStripArray[stripID];

        //        // if so, try to extract all the required parameters
        //        for (int param = 0; param < SampleStrip::TotalNumParams; ++param)
        //        {
        //            // check that this is a parameter that we load
        //            const bool doWeLoadParam = SampleStrip::isParamSaved(param);
        //            if (!doWeLoadParam) continue;

        //            // what name is this parameter?
        //            const String paramName = SampleStrip::getParameterName(param);

        //            // check that it can be found in the XML
        //            if (strip->hasAttribute(paramName))
        //            {
        //                // find its type
        //                const int paramType = SampleStrip::getParameterType(param);

        //                // DBG("id: " << stripID << " " << paramName << " " << param << " " << paramType);



        //            } // end of if(hasAtttribute)
        //            else
        //            {
        //                DBG("Param " << paramName << " not found. Doing nothing");
        //                // TODO: load default
        //                // OR do nothing
        //            }

        //        } // end loop over expected parameters

        //        // we have loaded this strip's parameters so can stop searching
        //        break;

        //    } // end of loop over preset entries

        //}// end of samplestrip loop

        // let the GUI know that we have reloaded
        processor->sendChangeMessage();
    }

    // if we've got to here we have sucessfully loaded
    return true;

}

bool Preset::loadSetlist(XmlElement * setlistToLoad,
                         mlrVSTAudioProcessor * processor,
                         GlobalSettings *gs)
{
    // clear existing setlist?
    processor->clearSetlist();
    processor->clearPresetList();

    // loop over each preset in the setlist
    forEachXmlChildElementWithTagName(*setlistToLoad, presetToLoad, "preset")
    {
        // try to load the preset
        const bool loadSucess = loadPreset(presetToLoad, processor, gs);

        DBG("tried: " << presetToLoad->getAttributeValue(0));

        // add the preset (replacing those with the same name)
        // to the preset list
        const bool replaceExisting = true;
        processor->addPreset(presetToLoad, replaceExisting);

    }

    return false;
}