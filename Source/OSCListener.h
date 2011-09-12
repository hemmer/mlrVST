#include "../JuceLibraryCode/JuceHeader.h"

// OSC send includes
//#include "osc/OscOutboundPacketStream.h"
//#include "ip/IpEndpointName.h"
// OSC receive includes
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
// OSC shared includes
#include "ip/UdpSocket.h"


using namespace osc;

class OscListener :
    public OscPacketListener,
    public Thread
{
    public:

        // Constructor
        OscListener() : Thread("OscListener Thread"),
            port(8000),
            s(IpEndpointName("localhost", port), this)
        {

        }

        ~OscListener()
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


    private:
        int port;
        UdpListeningReceiveSocket s;

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
                ReceivedMessageArgumentStream args = m.ArgumentStream();
                ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
                DBG(m.AddressPattern());

                if(strcmp(m.AddressPattern(), "/mlrvst/press") == 0)
                {
                    // example #1:
                    osc::int32 a1, a2, a3;
                    args >> a1 >> a2 >> a3 >> osc::EndMessage;

                    DBG("received '/mlrvst' message with arguments: " << a1 << " " << a2 << " " << a3 << "\n");
                }

            }
            catch(Exception& e)
            {
                DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
            }
        }

};

