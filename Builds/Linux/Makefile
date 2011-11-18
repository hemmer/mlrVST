# Automatically generated makefile, created by the Jucer
# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(TARGET_ARCH),)
  TARGET_ARCH := -march=native
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../oscpack" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
  CXXFLAGS += $(CFLAGS) -DOSC_HOST_LITTLE_ENDIAN
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../../Documents/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../oscpack" -I "../../JuceLibraryCode"
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../oscpack" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -fPIC -Os
  CXXFLAGS += $(CFLAGS) -DOSC_HOST_LITTLE_ENDIAN
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../../Documents/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../oscpack" -I "../../JuceLibraryCode"
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/AudioSample_c5634f81.o \
  $(OBJDIR)/PluginProcessor_a059e380.o \
  $(OBJDIR)/SampleStrip_9922baef.o \
  $(OBJDIR)/PresetPanel_1f2c0a46.o \
  $(OBJDIR)/SettingsPanel_89b72282.o \
  $(OBJDIR)/SampleStripControl_45995bf8.o \
  $(OBJDIR)/PluginEditor_94d4fb09.o \
  $(OBJDIR)/mlrVSTLookAndFeel_e70f60bb.o \
  $(OBJDIR)/OSCHandler_bb1eaf54.o \
  $(OBJDIR)/IpEndpointName_d4cc3aed.o \
  $(OBJDIR)/NetworkingUtils_571a8d2d.o \
  $(OBJDIR)/UdpSocket_34dc4264.o \
  $(OBJDIR)/OscOutboundPacketStream_595b04c5.o \
  $(OBJDIR)/OscPrintReceivedElements_db1e1922.o \
  $(OBJDIR)/OscReceivedElements_b8801585.o \
  $(OBJDIR)/OscTypes_b02e2cd6.o \
  $(OBJDIR)/juce_VST_Wrapper_812bffb1.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/JuceLibraryCode1_682c927f.o \
  $(OBJDIR)/JuceLibraryCode2_683aaa00.o \
  $(OBJDIR)/JuceLibraryCode3_6848c181.o \
  $(OBJDIR)/JuceLibraryCode4_6856d902.o \

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

$(OBJDIR)/PresetPanel_1f2c0a46.o: ../../Source/PresetPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PresetPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SettingsPanel_89b72282.o: ../../Source/SettingsPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SettingsPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SampleStripControl_45995bf8.o: ../../Source/SampleStripControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SampleStripControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginEditor_94d4fb09.o: ../../Source/PluginEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/mlrVSTLookAndFeel_e70f60bb.o: ../../Source/mlrVSTLookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling mlrVSTLookAndFeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OSCHandler_bb1eaf54.o: ../../Source/OSCHandler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OSCHandler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/IpEndpointName_d4cc3aed.o: ../../oscpack/ip/IpEndpointName.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling IpEndpointName.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/NetworkingUtils_571a8d2d.o: ../../oscpack/ip/win32/NetworkingUtils.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling NetworkingUtils.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/UdpSocket_34dc4264.o: ../../oscpack/ip/win32/UdpSocket.cpp
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

$(OBJDIR)/juce_VST_Wrapper_812bffb1.o: ../../../../../src/audio/plugin_client/VST/juce_VST_Wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST_Wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode1_682c927f.o: ../../JuceLibraryCode/JuceLibraryCode1.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode1.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode2_683aaa00.o: ../../JuceLibraryCode/JuceLibraryCode2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode2.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode3_6848c181.o: ../../JuceLibraryCode/JuceLibraryCode3.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode3.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode4_6856d902.o: ../../JuceLibraryCode/JuceLibraryCode4.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode4.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
