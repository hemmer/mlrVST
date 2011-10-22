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

private:
    // incoming messages
    int incomingPort;
    UdpListeningReceiveSocket s;
    mlrVSTAudioProcessor * const parent;

    // outgoing messages
    char buffer[1536];
    osc::OutboundPacketStream p;
    UdpTransmitSocket transmitSocket;

public:

    // Constructor
    OSCHandler(mlrVSTAudioProcessor * const owner) :
        Thread("OscListener Thread"),
        incomingPort(8000), parent(owner),
        s(IpEndpointName("localhost", incomingPort), this),
        transmitSocket(IpEndpointName("localhost", 8080)), p(buffer, 1536)
    {
        
    }


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

protected:

    void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/);

};


#endif