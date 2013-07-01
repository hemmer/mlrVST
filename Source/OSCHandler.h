#ifndef __OSCHANDLER__
#define __OSCHANDLER__

#include "../JuceLibraryCode/JuceHeader.h"

// OSC send includes
#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"
// OSC receive includes
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
// OSC shared includes
#include "ip/UdpSocket.h"

#include "AudioSample.h"

// forward declaration
class mlrVSTAudioProcessor;

class OSCHandler :  public osc::OscPacketListener,
                    public Thread
{

public:

    // Constructor
    OSCHandler(const String &prefix, mlrVSTAudioProcessor * const owner);

    ~OSCHandler()
    {
        // stop the OSC Listener thread running
        s.AsynchronousBreak();

        // allow the thread 2 seconds to stop cleanly - should be plenty of time.
        stopThread(2000);
    }

    // Start the oscpack OSC Listener Thread
    // NOTE: s.Run() won't return unless we force it to with
    // s.AsynchronousBreak() as is done in the destructor
    void run()
    {
        s.Run();
    }

    void buttonPressCallback(const int &monomeCol, const int &monomeRow, const bool &state);
    void setLED(const int &row, const int &col, const int &val);
    void setRow(const int &row, const int &val);
    void clearGrid();
    void setPrefix(const String &prefix)
    {
        OSCPrefix = "/" + prefix + "/";
        ledStr = OSCPrefix + "led";
        ledRowStr = OSCPrefix + "led_row";
        ledClearStr = OSCPrefix + "clear";
        buttonPressMask = OSCPrefix + "press";

        DBG("prefix now: " << OSCPrefix);
    }

private:
    // incoming /////////////////////////////
    // for communication with PluginProcessor
    mlrVSTAudioProcessor * const parent;
    int incomingPort;
    UdpListeningReceiveSocket s;


    // outgoing messages ////////////////////
    char buffer[1536];                  // to store message data
    osc::OutboundPacketStream p;
    UdpTransmitSocket transmitSocket;

    // strings ////////////////////////////
    String OSCPrefix;                       // main prefix (/mlrvst/ by default)
    String ledStr, ledRowStr, ledClearStr;  // + "led", + "led_row", + "clear"
    String buttonPressMask;                 // + "press"

    void handleStripMessage(const int &stripID, const osc::ReceivedMessage& m);

    float getFloatOSCArg(const osc::ReceivedMessage& m);
    int getIntOSCArg(const osc::ReceivedMessage& m);

    JUCE_LEAK_DETECTOR(OSCHandler);

protected:

    void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/);
};


#endif