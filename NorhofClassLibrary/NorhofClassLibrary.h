// NorhofClassLibrary.h

#pragma once

using namespace System;

namespace NorhofClassLibrary {

	public ref class NorhofDevice
	{
		// TODO: Add your methods for this class here.
		public:
			NorhofDevice(String^ name, short port_name);
			~NorhofDevice();
			Pump913drv::ConnectorClass^ gConn;
			Pump913drv::PumpMonitor^ gDrv;

			String^ scpi_query(String^ scpi_cmd);
			bool is_connected();

			virtual property String^ name {String^ get(); };
			virtual property String^ idn {String^ get(); };
			virtual property String^ serial_number {String^ get(); };
			virtual property String^ pump_status {String^ get(); };
			virtual property String^ pump_mode {String^ get(); };
			virtual property int control { int get(); void set(int value); };
			virtual property double temp { double get(); void set(double value); };
			virtual property double flow { double get(); void set(double value); };
			virtual property double vessel_temp { double get(); };
		    virtual property double pressure_in { double get(); };
			virtual property double extrasn_temp { double get(); };

		private:
			bool m_connected;
			short m_port_num;
			String^ m_name;

			String^ ExecSCPI(String^ cmd, String^ arg);
	};
}
