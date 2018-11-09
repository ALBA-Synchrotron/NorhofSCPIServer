using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

using NorhofClassLibrary;

namespace NorhofSyncServerSocket
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

                    if (server_name == "SCPIServer" && server_type == "sync")
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

    public class SynchronousSocketListener
    {
        // Incoming data from the client.  
        public static string data = null;
        public static Connection con = new Connection();
        public static NorhofDevice device = new NorhofDevice(con.server_name, con.serial_com);

        public static void StartListening()
        {
            // Data buffer for incoming data.  
            byte[] bytes = new Byte[1024];

            // Establish the local endpoint for the socket.  
            // Dns.GetHostName returns the name of the   
            // host running the application.  
            IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList[2];
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 11000);

            // Create a TCP/IP socket.  
            Socket listener = new Socket(ipAddress.AddressFamily,
                SocketType.Stream, ProtocolType.Tcp);

            // Bind the socket to the local endpoint and   
            // listen for incoming connections.  
            try
            {
                listener.Bind(localEndPoint);
                listener.Listen(10);
                Console.WriteLine("Norhof {0} {1} started, Port: {2}, COM: {3}", con.server_type, con.server_name, con.port, con.serial_com);
                Console.WriteLine("waiting for incoming connections...");
                Socket handler;
                // Start listening for connections.  
                while (true)
                {
                    // Program is suspended while waiting for an incoming connection.  
                    handler = listener.Accept();
                    data = null;
                    String answer;
                    // An incoming connection needs to be processed.  
                    while (true)
                    {
                        while (true)
                        {
                            int bytesRec = handler.Receive(bytes);
                            data += Encoding.ASCII.GetString(bytes, 0, bytesRec);
                            if (data.IndexOf("\n") > -1)
                            {
                                break;
                            }
                        }
                        // Parse command and return answer.
                        answer = ParseAsSCPI(handler, data);
                        // Show the data on the console.  
                        // Console.WriteLine("Text received : {0}", data);

                        // Echo the data back to the client.  
                        byte[] msg = Encoding.ASCII.GetBytes(answer + "\n");

                        handler.Send(msg);
                        data = "";
                    }

                    //handler.Shutdown(SocketShutdown.Both);
                    //handler.Close();
                }

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            Console.WriteLine("\nPress ENTER to continue...");
            Console.Read();

        }

        private static String ParseAsSCPI(Socket handler, String data)
        {
            // remove \n delimiter
            data = data.Substring(0, data.Length - 1);
            String answer = device.scpi_query(data);

            // Convert bytes sent to ASCII for debugging.
            //Console.WriteLine("SCPI answer received: {0}", answer);
            return answer;
            // Convert the string data to byte data using ASCII encoding.  
            //        byte[] byteData = Encoding.ASCII.GetBytes(answer);

            // Return answer to the remote device  
            //        handler.BeginSend(byteData, 0, byteData.Length, 0,
            //            new AsyncCallback(SendCallback), handler);
        }

        public static int Main(String[] args)
        {
            StartListening();
            return 0;
        }
    }
}
