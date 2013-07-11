# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ~/SDKs/vstsdk2.4 -I ../../JuceLibraryCode -I ../../oscpack/
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
  CXXFLAGS += $(CFLAGS) -DOSC_HOST_LITTLE_ENDIAN
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -shared -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ~/SDKs/vstsdk2.4 -I ../../JuceLibraryCode -I ../../oscpack/
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ~/SDKs/vstsdk2.4 -I ../../JuceLibraryCode -I ../../oscpack/
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -fPIC -Os
  CXXFLAGS += $(CFLAGS) -DOSC_HOST_LITTLE_ENDIAN
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -shared -fvisibility=hidden -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ~/SDKs/vstsdk2.4 -I ../../JuceLibraryCode -I ../../oscpack/
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

OBJECTS := \
  $(OBJDIR)/AudioSample_c5634f81.o \
  $(OBJDIR)/PluginProcessor_a059e380.o \
  $(OBJDIR)/SampleStrip_9922baef.o \
  $(OBJDIR)/PatternRecording_c3abd24a.o \
  $(OBJDIR)/SampleStripControl_45995bf8.o \
  $(OBJDIR)/PatternStripControl_22fcf2b6.o \
  $(OBJDIR)/HintOverlay_866575aa.o \
  $(OBJDIR)/FreeTypeAmalgam_f9f8342c.o \
  $(OBJDIR)/FreeTypeFaces_5f2c9c86.o \
  $(OBJDIR)/TextDragSlider_db5742cb.o \
  $(OBJDIR)/CustomArrowButton_7802bccb.o \
  $(OBJDIR)/timedButton_8bab4a.o \
  $(OBJDIR)/PresetListTable_856af992.o \
  $(OBJDIR)/PresetPanel_1f2c0a46.o \
  $(OBJDIR)/SetlistTable_bd90937.o \
  $(OBJDIR)/SettingsPanel_89b72282.o \
  $(OBJDIR)/MappingPanel_5b8ca75f.o \
  $(OBJDIR)/mlrVSTGUI_eeb94ef8.o \
  $(OBJDIR)/mlrVSTLookAndFeel_e70f60bb.o \
  $(OBJDIR)/MappingEngine_21233411.o \
  $(OBJDIR)/Utilities_707bb76b.o \
  $(OBJDIR)/OSCHandler_bb1eaf54.o \
  $(OBJDIR)/IpEndpointName_d4cc3aed.o \
  $(OBJDIR)/NetworkingUtils_921f9941.o \
  $(OBJDIR)/UdpSocket_2b50ed78.o \
  $(OBJDIR)/OscOutboundPacketStream_595b04c5.o \
  $(OBJDIR)/OscPrintReceivedElements_db1e1922.o \
  $(OBJDIR)/OscReceivedElements_b8801585.o \
  $(OBJDIR)/OscTypes_b02e2cd6.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_audio_basics_2442e4ea.o \
  $(OBJDIR)/juce_audio_devices_a4c8a728.o \
  $(OBJDIR)/juce_audio_formats_d349f0c8.o \
  $(OBJDIR)/juce_audio_processors_44a134a2.o \
  $(OBJDIR)/juce_core_aff681cc.o \
  $(OBJDIR)/juce_data_structures_bdd6d488.o \
  $(OBJDIR)/juce_events_79b2840.o \
  $(OBJDIR)/juce_graphics_c8f1e7a4.o \
  $(OBJDIR)/juce_gui_basics_a630dd20.o \
  $(OBJDIR)/juce_gui_extra_7767d6a8.o \
  $(OBJDIR)/juce_PluginUtilities_e2e19a34.o \
  $(OBJDIR)/juce_VST_Wrapper_bb62e93d.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking mlrVST
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning mlrVST
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

strip:
	@echo Stripping mlrVST
	-@strip --strip-unneeded $(OUTDIR)/$(TARGET)

$(OBJDIR)/AudioSample_c5634f81.o: ../../Source/AudioSample.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioSample.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SampleStrip_9922baef.o: ../../Source/SampleStrip.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SampleStrip.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PatternRecording_c3abd24a.o: ../../Source/PatternRecording.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PatternRecording.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SampleStripControl_45995bf8.o: ../../Source/SampleStripControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SampleStripControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PatternStripControl_22fcf2b6.o: ../../Source/PatternStripControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PatternStripControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HintOverlay_866575aa.o: ../../Source/HintOverlay.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling HintOverlay.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FreeTypeAmalgam_f9f8342c.o: ../../Source/Fonts/FreeTypeAmalgam.c
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FreeTypeAmalgam.c"
	@$(CC) $(CFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FreeTypeFaces_5f2c9c86.o: ../../Source/Fonts/FreeTypeFaces.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FreeTypeFaces.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TextDragSlider_db5742cb.o: ../../Source/TextDragSlider.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TextDragSlider.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CustomArrowButton_7802bccb.o: ../../Source/CustomArrowButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CustomArrowButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/timedButton_8bab4a.o: ../../Source/timedButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling timedButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PresetListTable_856af992.o: ../../Source/PresetListTable.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PresetListTable.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PresetPanel_1f2c0a46.o: ../../Source/PresetPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PresetPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SetlistTable_bd90937.o: ../../Source/SetlistTable.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SetlistTable.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SettingsPanel_89b72282.o: ../../Source/SettingsPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SettingsPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MappingPanel_5b8ca75f.o: ../../Source/MappingPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MappingPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/mlrVSTGUI_eeb94ef8.o: ../../Source/mlrVSTGUI.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling mlrVSTGUI.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/mlrVSTLookAndFeel_e70f60bb.o: ../../Source/mlrVSTLookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling mlrVSTLookAndFeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MappingEngine_21233411.o: ../../Source/MappingEngine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MappingEngine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Utilities_707bb76b.o: ../../Source/Utilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Utilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OSCHandler_bb1eaf54.o: ../../Source/OSCHandler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OSCHandler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/IpEndpointName_d4cc3aed.o: ../../oscpack/ip/IpEndpointName.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling IpEndpointName.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/NetworkingUtils_921f9941.o: ../../oscpack/ip/NetworkingUtils.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling NetworkingUtils.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/UdpSocket_2b50ed78.o: ../../oscpack/ip/UdpSocket.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling UdpSocket.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OscOutboundPacketStream_595b04c5.o: ../../oscpack/osc/OscOutboundPacketStream.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OscOutboundPacketStream.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OscPrintReceivedElements_db1e1922.o: ../../oscpack/osc/OscPrintReceivedElements.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OscPrintReceivedElements.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OscReceivedElements_b8801585.o: ../../oscpack/osc/OscReceivedElements.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OscReceivedElements.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OscTypes_b02e2cd6.o: ../../oscpack/osc/OscTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OscTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_basics_2442e4ea.o: ../../JuceLibraryCode/modules/juce_audio_basics/juce_audio_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_devices_a4c8a728.o: ../../JuceLibraryCode/modules/juce_audio_devices/juce_audio_devices.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_devices.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_formats_d349f0c8.o: ../../JuceLibraryCode/modules/juce_audio_formats/juce_audio_formats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_formats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_processors_44a134a2.o: ../../JuceLibraryCode/modules/juce_audio_processors/juce_audio_processors.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_processors.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_aff681cc.o: ../../JuceLibraryCode/modules/juce_core/juce_core.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_data_structures_bdd6d488.o: ../../JuceLibraryCode/modules/juce_data_structures/juce_data_structures.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_events_79b2840.o: ../../JuceLibraryCode/modules/juce_events/juce_events.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_graphics_c8f1e7a4.o: ../../JuceLibraryCode/modules/juce_graphics/juce_graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_basics_a630dd20.o: ../../JuceLibraryCode/modules/juce_gui_basics/juce_gui_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_extra_7767d6a8.o: ../../JuceLibraryCode/modules/juce_gui_extra/juce_gui_extra.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_PluginUtilities_e2e19a34.o: ../../JuceLibraryCode/modules/juce_audio_plugin_client/utility/juce_PluginUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_PluginUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VST_Wrapper_bb62e93d.o: ../../JuceLibraryCode/modules/juce_audio_plugin_client/VST/juce_VST_Wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST_Wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
