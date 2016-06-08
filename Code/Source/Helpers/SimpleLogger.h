#pragma once

#include <Coherent/LibrariesGT/Logging/ILogHandler.h>

class SimpleLogger : public Coherent::LoggingGT::ILogHandler
{
public:
	// The current implementation of the logger takes a log message and routes
	// it to the output window.
	// The log messages are separated in several categories and the desired
	// level of verbosity is passed as a parameter when
	// initializing the UISystem
	virtual void WriteLog(Coherent::LoggingGT::Severity sev,
		const char* msg,
		size_t) COHERENT_OVERRIDE
	{
		char buf[512];
		const char* severity = "";
		switch (sev)
		{
		case Coherent::LoggingGT::Trace:
			severity = "Trace";
			break;
		case Coherent::LoggingGT::Debug:
			severity = "Debug";
			break;
		case Coherent::LoggingGT::Info:
			severity = "Info";
			break;
		case Coherent::LoggingGT::Warning:
			severity = "Warning";
			break;
		case Coherent::LoggingGT::Error:
			severity = "Error";
			break;
		case Coherent::LoggingGT::AssertFailure:
			severity = "AssertFailure";
			break;
		}
		_snprintf(buf, sizeof(buf), "Log (%s): %s\r\n", severity, msg);
		WriteMessage(buf);
	}

		virtual void Assert(const char* msg) COHERENT_OVERRIDE
	{
		char buf[512];
		_snprintf(buf, sizeof(buf), "Assert: %s\r\n", msg);
		WriteMessage(buf);
	}

		void WriteMessage(const char* message)
	{
		CryLogAlways("[COHERENT]: " + string(message));
	}

#pragma warning(pop)

};