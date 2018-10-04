Last revision: 04-Oct-2018

Norhof SCPI Server is a Server Socket application used to control and monitor the LN2 pump from Norhof based on the pump913drv.1.2.

The solution is composed by three projects:

* NorhofClassLibrary: The class module communication library in Visual C++. Communicates with the norhoff driver and exports symbols and functionallity.
* NorhofServerSocket: Implementation of an asynchronous server socket on C# following the SCPI protocol.
* Norhof SCPI Server: ACPI seerver application project for deplyment of the solution (Installer).

The solution has the Norhof drivers as external dependency, but it is not explicitely indicated in the solution.
You are the responsible for having the Norhof drivers installed in the target machine. The Norhof drivers are installed by default when installing any of the native applications supplied by Norhof.

In order to install the SCPI server you only need to execute the installer:

    NorhofPumpController\Norhof SCPI Server\Release\Norhof SCPI Server.msi

and follow the instructions.
