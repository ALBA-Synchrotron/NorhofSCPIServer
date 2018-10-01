// NorhofCLRClient.cpp : main project file.

#include "stdafx.h"

using namespace System;
using namespace NorhofClassLibrary;

int main(array<System::String ^> ^args)
{
    Console::WriteLine(L"Hello World");
	NorhofDevice^ d = gcnew NorhofDevice("my device", 3);
	Threading::Thread::Sleep(3000);
	Console::WriteLine(d->name);
	Console::WriteLine(d->is_connected());
	Console::WriteLine(d->info);
	Console::WriteLine(d->serial_number);
	Console::WriteLine(System::String::Format("Pump tatus = {0}", d->pump_status));
	Console::WriteLine(System::String::Format("Pump mode = {0}", d->pump_mode));
	Console::WriteLine(System::String::Format("Temperature = {0}", d->temp));
	Console::WriteLine(System::String::Format("Flow = {0}", d->flow));
	return 0;
}
