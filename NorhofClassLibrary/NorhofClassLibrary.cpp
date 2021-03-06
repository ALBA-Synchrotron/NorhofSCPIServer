// This is the main DLL file.

#include "stdafx.h"

#include "NorhofClassLibrary.h"

NorhofClassLibrary::NorhofDevice::NorhofDevice(String^ name, short port_num) : m_name(name), m_port_num(port_num) {

	gConn = gcnew Pump913drv::ConnectorClass();
	gDrv = gConn->PumpMonitor;
	m_connected = gDrv->CommPort[m_port_num];
}
NorhofClassLibrary::NorhofDevice::~NorhofDevice()
{
	gDrv = nullptr;
	gConn = nullptr;
}
String^ NorhofClassLibrary::NorhofDevice::name::get()
{
	return this->m_name;
}

String^ NorhofClassLibrary::NorhofDevice::idn::get()
{
	array<String^>^ delimiter = { ":" };
	array<String^>^ space = { " " };
	array<String^>^ cr = { "\r" };
	array<String^>^ words;

	String^ token = this->gDrv->PumpInfo;
	words = token->Split(delimiter, System::StringSplitOptions::RemoveEmptyEntries);
	String^ company = words[1]->Split(space, System::StringSplitOptions::RemoveEmptyEntries)[0];
	String^ model = words[1]->Split(space, System::StringSplitOptions::RemoveEmptyEntries)[1];
	String^ version = words[2]->Split(cr, System::StringSplitOptions::RemoveEmptyEntries)[0]->Remove(0, 1)->Remove(9, 9);
	String^ date = words[3]->Split(cr, System::StringSplitOptions::RemoveEmptyEntries)[0]->Remove(0, 4)->Remove(5, 1);
	return (company + "," + model + "," + version + "- " + date + ",");
}

String^ NorhofClassLibrary::NorhofDevice::serial_number::get()
{
	Object^ objSerialNumber = this->gDrv->PumpSerialNumber;
	return objSerialNumber->ToString();
}

bool NorhofClassLibrary::NorhofDevice::is_connected()
{
	return m_connected;
}

// Returns the pump status as a decimal value
String^ NorhofClassLibrary::NorhofDevice::pump_status::get()
{
	Object^ status = this->gDrv->PumpRegister[Pump913drv::regRequest::regPumpStatus];
	return status->ToString();
}

// Returns the pumping mode as Hexadecimal value (RS-232 control is F = 15).
String^ NorhofClassLibrary::NorhofDevice::pump_mode::get()
{
	Object^ status = this->gDrv->PumpRegister[Pump913drv::regRequest::regPumpMode];
	return System::Convert::ToString(System::Convert::ToInt32(status), 16);
}

// Returns the pump operation control (0=stanby, 3=pumping)
int NorhofClassLibrary::NorhofDevice::control::get()
{
	Object^ flow = this->gDrv->RS232Get[Pump913drv::RScon::rscControl];
	return System::Convert::ToInt32(flow->ToString());
}

void NorhofClassLibrary::NorhofDevice::control::set(int value)
{
	bool success = this->gDrv->RS232Set[Pump913drv::RScon::rscControl, value];
	if (!success)
	{
		throw;
	}
	
}

// Returns the pressure in bar.
double NorhofClassLibrary::NorhofDevice::pressure_in::get()
{
	return System::Convert::ToDouble(
				this->gDrv->ADC::get(Pump913drv::Sensor::snPressure, Pump913drv::Result::rsMbar)
		);
}

// Returns the vessel in Celcius.
double NorhofClassLibrary::NorhofDevice::vessel_temp::get()
{
	return System::Convert::ToDouble(
				this->gDrv->ADC::get(Pump913drv::Sensor::snVessel, Pump913drv::Result::rsTemprature)
		);
}

// Returns the extra sensor in Celcius.
double NorhofClassLibrary::NorhofDevice::extrasn_temp::get()
{

	return System::Convert::ToDouble(
		this->gDrv->ADC::get(Pump913drv::Sensor::snextra, Pump913drv::Result::rsTemprature)
		);

}

// Returns the set temperature in Celcius.
double NorhofClassLibrary::NorhofDevice::temp::get()
{
	return System::Convert::ToDouble(this->gDrv->RS232Get[Pump913drv::RScon::rscTemprature]);
}

void NorhofClassLibrary::NorhofDevice::temp::set(double temp_set)
{
	bool success = this->gDrv->RS232Set[Pump913drv::RScon::rscTemprature, temp_set];
}

// Returns the flow set in mbar (pressure).
double NorhofClassLibrary::NorhofDevice::flow::get()
{
	return System::Convert::ToDouble(this->gDrv->RS232Get[Pump913drv::RScon::rscFlow]);
}

void NorhofClassLibrary::NorhofDevice::flow::set(double flow_set)
{
	bool success = this->gDrv->RS232Set[Pump913drv::RScon::rscFlow, flow_set];
}

array<String^>^ ParseCommand(String^ cmd)
{
	char cmd_delim = ':';
	array<String^>^ cmds = cmd->Split(cmd_delim);
	return cmds;
}

int GetCommandRank(String^ cmd)
{
	array<String^>^ cmds = ParseCommand(cmd);
	return cmds->Length -1;
}

String^ NorhofClassLibrary::NorhofDevice::ExecSCPI(String^ cmd, String^ arg)
{

	String^ result;
	//Console::WriteLine("Command {0} has rank {1}.", cmd, GetCommandRank(cmd));
	if (GetCommandRank(cmd) == 0)
	{
		if (cmd->StartsWith("*IDN")) { result = idn; }
	}
	else if (GetCommandRank(cmd) == 1 && arg != nullptr)
	{
		try
		{
			if (cmd->StartsWith(":TEMP")) { temp = System::Convert::ToDouble(arg); }
			else if (cmd->StartsWith(":FLOW")) { flow = System::Convert::ToDouble(arg); }
			else
			{
				Console::WriteLine("Non implemented command or invalid format.");
				result = false.ToString();
			}

			result = true.ToString();
		}
		catch (...)
		{
			result = false.ToString();
		}
	}
	else if (GetCommandRank(cmd) == 2 && arg == nullptr)
	{
		array<String^>^ cmds = ParseCommand(cmd);
		
		if (cmds[1]->StartsWith("SYS"))
		{
			if (cmds[2] == "NAME") { result = name; }
			else if (cmds[2] == "SN") { result = serial_number; }
			else if (cmds[2] == "STATUS") { result = pump_status; }
		}
		else if (cmds[1]->StartsWith("MEAS"))
		{
			if (cmds[2] == "TEMP") { result = temp.ToString(); }
			else if (cmds[2] == "FLOW") { result = flow.ToString(); }
			else if (cmds[2] == "VESSEL_TEMP") { result = vessel_temp.ToString(); }
			else if (cmds[2] == "EXTRASN_TEMP") { result = extrasn_temp.ToString(); }
            else if (cmds[2] == "PRESSURE_IN") { result = pressure_in.ToString(); }
		}
		else if (cmds[1]->StartsWith("PUMP"))

			if (cmds[2] == "MODE") { result = pump_mode; }
			else if (cmds[2] == "CONTROL") { result = control.ToString(); }
			else
			{
				Console::WriteLine("Non implemented command or invalid format.");
				result = "None";
			}
	}
	else if (GetCommandRank(cmd) == 2 && arg != nullptr)
	{
		array<String^>^ cmds = ParseCommand(cmd);

		if (cmds[1]->StartsWith("PUMP"))
		{
	        if (cmds[2] == "CONTROL") { control = System::Convert::ToInt32(arg); }
			else
			{
				Console::WriteLine("Non implemented command or invalid format.");
				result = "None";
			}

			result = true.ToString();
		}

	}
	return result;
}

String^ NorhofClassLibrary::NorhofDevice::scpi_query(String^ scpi_cmd)
{
	array<String^>^ cmd_delim = { ";" };
	String^ arg;
	String^ answer;
	String^ result;

	Console::WriteLine("Processing SCPI command(s): {0}", scpi_cmd);
	
	// Support for multiple SCPI commands
	array<String^>^ cmds = scpi_cmd->Split(cmd_delim, System::StringSplitOptions::RemoveEmptyEntries);
	Console::WriteLine("Command(s): <{0}>", cmds);
	for each (String^ cmd in cmds)
	{
		//Console::WriteLine("*** Processing SCPI command {0}",cmd);
		// Protect from bad-formatted commands
		if (!(cmd[0].Equals(':') || cmd[0].Equals('*')))
		{
			Console::WriteLine("Bad-formatted command!");
			Console::WriteLine(cmd);
			return "None";
		}

		bool query = false;
		// is a query? remove question mark
		if (cmd[cmd->Length - 1].Equals('?'))
		{
			cmd = cmd->Substring(0, cmd->Length - 1);
			query = true;

		}
		else // Maybe has argument?
		{
			array<String^>^ parts = cmd->Split(' ');
			try
			{
				arg = parts[1];
				cmd = parts[0];
				//Console::WriteLine("Command = {0}", cmd);
				//Console::WriteLine("Argument = {0}", arg);
			}
			catch (Exception^ e)
			{
				arg = nullptr;
				//Console::WriteLine("No argument found");
			}
		}

		//Console::WriteLine("Command: {0}", cmd);

		// Execute command and store results in String
		result = ExecSCPI(cmd, arg);
		answer += result + ";";
	}
	
	if (cmds->Length == 1)
		answer = answer->Substring(0, answer->Length - 1);
	Console::WriteLine("Answer(s): <{0}>", answer);
	return answer;
}
