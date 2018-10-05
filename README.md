Last revision: 05-Oct-2018

Norhof SCPI Servers are Server Socket applications used to control and monitor the LN2 pump from Norhof based on the pump913drv.1.2.
Two application are distributed with the same installer: synchronous and asynchronous server version.

The synchronous version is designed to be integrated with the Skippy Tango DS (https://github.com/srgblnch/skippy/).

The solution is composed by four projects:

* NorhofClassLibrary: The class module communication library in Visual C++. Communicates with the norhoff driver and exports symbols and functionallity.
* NorhofAsyncServerSocket: Implementation of an asynchronous server socket on C# following the SCPI protocol.
* NorhofSyncServerSocket: Implementation of an synchronous server socket on C# following the SCPI protocol.
* Norhof SCPI Server: Application project for deployment of the solutions (It installs both server versions).

The solution has the Norhof driver as external dependency, but it is not explicitely indicated in the solution.
So, you are the responsible for having the Norhof drivers installed in the target machine. The Norhof drivers are installed by default when installing any of the native applications supplied by Norhof.

In order to install the SCPI servers, you only need to execute the windows installer:

    NorhofPumpController\Norhof SCPI Server\Release\Norhof SCPI Server.msi

This will install the servers (and all dependencies) in a default location:

    C:\Program Files (x86)\ALBA\Norhof SCPI Server

The configuration for each of the servers is defines in the _conf.xml_ file created in the same intall location.

Configuration file example (for the synchrobous version):

    <NorhofPump>
        <SCPIServer type="sync" port="11000" serial_com="3">
        </SCPIServer>
    </NorhofPump>

In order to run the asynchronous version, change the type to _async_.
