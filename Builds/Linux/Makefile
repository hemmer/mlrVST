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
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../../Documents/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../JuceLibraryCode"
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -fPIC -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../../Documents/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "~/SDKs/vstsdk2.4" -I "/usr/include" -I "/usr/include/freetype2" -I "../../JuceLibraryCode"
  TARGET := mlrVST.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/AudioSample_c5634f81.o \
  $(OBJDIR)/ChannelProcessor_6fdb09d8.o \
  $(OBJDIR)/Zynth_e6f811a4.o \
  $(OBJDIR)/Zampler_20b72fb0.o \
  $(OBJDIR)/WaveformControl_fbaf07c1.o \
  $(OBJDIR)/PluginProcessor_a059e380.o \
  $(OBJDIR)/PluginEditor_94d4fb09.o \
  $(OBJDIR)/juce_VST_Wrapper_812bffb1.o \
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

$(OBJDIR)/ChannelProcessor_6fdb09d8.o: ../../Source/ChannelProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Zynth_e6f811a4.o: ../../Source/Zynth.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Zynth.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Zampler_20b72fb0.o: ../../Source/Zampler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Zampler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WaveformControl_fbaf07c1.o: ../../Source/WaveformControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WaveformControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginEditor_94d4fb09.o: ../../Source/PluginEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VST_Wrapper_812bffb1.o: ../../../../../src/audio/plugin_client/VST/juce_VST_Wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST_Wrapper.cpp"
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