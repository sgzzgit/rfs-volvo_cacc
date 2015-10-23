// WavetronixRead.cpp : main project file.

#include "stdafx.h"

using namespace System;
using namespace System::IO;
using namespace System::IO::Ports;


int main(array<System::String ^> ^args)
{
	System::DateTime dtNow;
	String^ dtNowString;

	dtNow = DateTime::Now;
	dtNowString = dtNow.Year.ToString() + "." + dtNow.Month.ToString() + "." + 
		 dtNow.Day.ToString() + "_" + dtNow.Hour.ToString() + "." + 
		 dtNow.Minute.ToString() + "." + dtNow.Second.ToString();

	String^ fileName = "wavetronix" + dtNowString + ".log";

	FileInfo^ fileInfo = gcnew FileInfo(fileName);
	StreamWriter^ dataFile = fileInfo->CreateText();;
	SerialPort^ port = gcnew SerialPort ();

	Console::WriteLine(L"Wavetronix data");


	try{

		port->PortName = "COM1";
		port->BaudRate = 9600;
		port->Parity = (Parity) 0;
		port->DataBits = 8;
		port->StopBits = (StopBits) 1;		
		// Timeout after 60 seconds.
		//port->ReadTimeout = 60000;

		port->Open();

		String^ writeMessage = "";
		writeMessage = "XT\r";
		//writeMessage = "X5\r";

		int index = 0;
		int indexData = 0;
			
		String^ messageString;
		char messageChar ;
		array<unsigned char>^ unsignedChar;
		array<unsigned char>^ unsignedCharData = gcnew array<unsigned char> (85);

		while(true){

			messageString = "";
			unsignedChar = gcnew array<unsigned char> (85);

			dtNow = DateTime::Now;
			dtNowString = dtNow.Year.ToString() + "/" + dtNow.Month.ToString() + "/" + 
				 dtNow.Day.ToString() + "_" + dtNow.Hour.ToString() + ":" + 
				 dtNow.Minute.ToString() + ":" + dtNow.Second.ToString() + "." + 
				 dtNow.Millisecond.ToString();

			port->Write(String::Format("{0}", writeMessage));			

			Console::Write("{0} ", dtNowString);
			dataFile->Write("{0} ", dtNowString);

			while(true){

				unsignedChar[index] = port->ReadByte();
			
				//messageString = System::Text::Encoding::ASCII->GetString(unsignedChar);
				messageString = unsignedChar[index].ToString();

				Console::Write("{0} ", messageString);
				dataFile->Write("{0} ", unsignedChar[index]);
				//dataFile->Write("{0} ", messageString);

				index++;
				if(index == 85){
		
					dataFile->WriteLine();
					break;
				}
			}
			index = 0;
		}
	}
    catch (Exception^ ex)
    {

		dataFile->Close();
		port->Close();

		Console::WriteLine("\n");
        Console::WriteLine(ex);
		Console::Read();
    }

	dataFile->Close();
	port->Close();

    return 0;
}
