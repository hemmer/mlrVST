#ifndef __OSCHANDLER__
#define __OSCHANDLER__

#include "../JuceLibraryCode/JuceHeader.h"

// OSC send includes
//#include "osc/OscOutboundPacketStream.h"
//#include "ip/IpEndpointName.h"
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
    int port;
    UdpListeningReceiveSocket s;
    mlrVSTAudioProcessor * const parent;

public:

    // Constructor
    OSCHandler(mlrVSTAudioProcessor * const owner) :
        Thread("OscListener Thread"),
        port(8000),
        s(IpEndpointName("localhost", port), this),
        parent(owner)
    {
        
    }

    //// Constructor
    //OSCHandler() :
    //    Thread("OscListener Thread"),
    //    port(8000),
    //    s(IpEndpointName("localhost", port), this)
    //{

    //}


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

    void sendMessage(const int &a1, const int &a2, const int &a3);


protected:

    void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& /*remoteEndpoint*/)
    {
        // a more complex scheme involving std::map or some other method of
        // processing address patterns could be used here
        // (see MessageMappingOscPacketListener.h for example). however, the main
        // purpose of this example is to illustrate and test different argument
        // parsing methods

        try
        {
            // argument stream, and argument iterator, used in different
            // examples below.
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            //DBG(m.AddressPattern());

            if (strcmp(m.AddressPattern(), "/mlrvst/press") == 0)
            {
                // example #1:
                osc::int32 a1, a2, a3;
                args >> a1 >> a2 >> a3 >> osc::EndMessage;
                sendMessage(a1, a2, a3);

                //DBG("received '/mlrvst' message with arguments: " << a1 << " " << a2 << " " << a3 << "\n");
            }

        }
        catch (osc::Exception& e)
        {
            DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
        }
    }

};


#endif