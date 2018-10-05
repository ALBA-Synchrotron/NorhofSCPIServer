using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

using NorhofClassLibrary;

namespace NorhofAsyncServerSocket
{
    public class Connection
    {
        public String server_name;
        public String server_type;
        public short port = 0;
        public short serial_com = 0;

        public Connection()
        {
            try
            {
                String filename = "conf.xml";
                XDocument conf = XDocument.Load(filename);
                
                foreach (XElement el in conf.Root.Elements())
                {
                    server_name = el.Name.ToString();
                    server_type = el.Attribute("type").Value;

                    if (server_name == "SCPIServer" && server_type == "async")
                    {
                        port = System.Convert.ToInt16(el.Attribute("port").Value);
                        serial_com = System.Convert.ToInt16(el.Attribute("serial_com").Value);
                        break;
                    }
                    
                }
                if (port == 0)
                {
                    throw new Exception("Invalid server type. Valid values are sync or async.");
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception when loading configuration file\n{0}", e);
                System.Environment.Exit(1);
            }
        }
    }

    // State object for reading client data asynchronously  
    public class StateObject
    {
        // Client  socket.  
        public Socket workSocket = null;
        // Size of receive buffer.  
        public const int BufferSize = 1024;
        // Receive buffer.  
        public byte[] buffer = new byte[BufferSize];
        // Received data string.  
        public StringBuilder sb = new StringBuilder();
    }

    public class AsynchronousSocketListener
    {
        public static ManualResetEvent allDone = new ManualResetEvent(false);
        public static Connection con = new Connection();
        public static NorhofDevice device = new NorhofDevice(con.server_name, con.serial_com);

        public AsynchronousSocketListener() {}

        public static void StartListening()
        {
            // Establish the local endpoint for the socket.  
            // The DNS name of the computer  
            // running the listener is "host.contoso.com".  
            IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList[2];
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, con.port);

            // Create a TCP/IP socket.  
            Socket listener = new Socket(ipAddress.AddressFamily,
                SocketType.Stream, ProtocolType.Tcp);

            // Bind the socket to the local endpoint and listen for incoming connections.  
            try
            {
                listener.Bind(localEndPoint);
                listener.Listen(100);
                Console.WriteLine("Norhof {0} {1} started, Port: {2}, COM: {3}", con.server_type, con.server_name, con.port, con.serial_com);
                Console.WriteLine("waiting for incoming connections...") ;

                while (true)
                {
                    // Set the event to nonsignaled state.  
                    allDone.Reset();
                    // Start an asynchronous socket to listen for connections.  
                    listener.BeginAccept(
                        new AsyncCallback(AcceptCallback), listener);
                    // Wait until a connection is made before continuing.  
                    allDone.WaitOne();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            Console.WriteLine("\nPress ENTER to continue...");
            Console.Read();

        }

        public static void AcceptCallback(IAsyncResult ar)
        {
            // Signal the main thread to continue.  
            allDone.Set();

            // Get the socket that handles the client request.  
            Socket listener = (Socket)ar.AsyncState;
            Socket handler = listener.EndAccept(ar);

            // Create the state object.  
            StateObject state = new StateObject();
            state.workSocket = handler;
            handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReadCallback), state);
        }

        public static void ReadCallback(IAsyncResult ar)
        {
            String content = String.Empty;

            // Retrieve the state object and the handler socket  
            // from the asynchronous state object.  
            StateObject state = (StateObject)ar.AsyncState;
            Socket handler = state.workSocket;
            // Read data from the client socket.   
            int bytesRead = handler.EndReceive(ar);
            if (bytesRead > 0)
            {
                // There  might be more data, so store the data received so far.  
                state.sb.Append(Encoding.ASCII.GetString(
                    state.buffer, 0, bytesRead));

                // Check for end-of-file tag. If it is not there, read   
                // more data.  
                content = state.sb.ToString();
                if (content.IndexOf("\n") > -1)
                {
                    // All the data has been read from the   
                    // client.
                    ParseAsSCPI(handler, content);
                }
                else
                {
                    // Not all data received. Get more.  
                    handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                    new AsyncCallback(ReadCallback), state);
                }
            }
        }

        private static void Send(Socket handler, String data)
        {
            // Convert the string data to byte data using ASCII encoding.  
            byte[] byteData = Encoding.ASCII.GetBytes(data);

            // Begin sending the data to the remote device.  
            handler.BeginSend(byteData, 0, byteData.Length, 0,
                new AsyncCallback(SendCallback), handler);
        }

        private static void SendCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.  
                Socket handler = (Socket)ar.AsyncState;

                // Complete sending the data to the remote device.  
                int bytesSent = handler.EndSend(ar);
                handler.Shutdown(SocketShutdown.Both);
                handler.Close();

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private static void ParseAsSCPI(Socket handler, String data)
        {
            // remove \n delimiter
            data = data.Substring(0, data.Length - 2);
            String answer = device.scpi_query(data);

            // Convert bytes sent to ASCII for debugging.
            Console.WriteLine("SCPI answer received: {0}", answer);
            
            // Convert the string data to byte data using ASCII encoding.  
            byte[] byteData = Encoding.ASCII.GetBytes(answer);

            // Return answer to the remote device  
            handler.BeginSend(byteData, 0, byteData.Length, 0,
                new AsyncCallback(SendCallback), handler);
        }
        
        public static int Main(String[] args)
        {
            StartListening();
            return 0;
        }
    }
}
