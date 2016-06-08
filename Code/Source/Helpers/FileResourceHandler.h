#pragma once

#include <Coherent/UIGT/ResourceHandler.h>
#include <string>
#include <fstream>

// Inherint the base ResouceHandler class and use a custom implementation
// of OnResourceRead to read files.
class FileResourceHandler : public Coherent::UIGT::ResourceHandler
{
public:
	virtual void OnResourceRead(
		const Coherent::UIGT::ResourceRequestUIGT* request,
		Coherent::UIGT::ResourceResponseUIGT* response)
		COHERENT_OVERRIDE
	{
		Coherent::UIGT::URLComponent hostComp;
		Coherent::UIGT::URLComponent pathComp;
		Coherent::UIGT::URLComponent queryComp;

		std::string asciiUrl(request->GetURL());
		const char* url = asciiUrl.c_str();

		if (!Coherent::UIGT::URLParse(url, &hostComp, &pathComp, &queryComp)) {
			assert(false && "Invalid url!");
			response->SignalFailure();
			return;
		}

		std::string pathStr(pathComp.Start, pathComp.Length);
		CryLogAlways("PATH STRING: %s", pathStr);
		unsigned decodedSize = 0;
		Coherent::UIGT::DecodeURLString(pathStr.c_str(), nullptr, &decodedSize);

		if (!decodedSize)
		{
			response->SignalFailure();
			response->Release();
			return;
		}

		std::unique_ptr<char[]> decodedStr(new char[decodedSize]);
		Coherent::UIGT::DecodeURLString(pathStr.c_str(), decodedStr.get(), &decodedSize);

		std::string decodedURL = decodedStr.get();

#if defined(COHERENT_PLATFORM_WIN)
		auto result = MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, decodedURL.c_str(), -1, 0, 0);

		if (result == ERROR_NO_UNICODE_TRANSLATION)
		{
			std::string error("The URL of the requested resource ");
			error += decodedURL;
			error += " contains invalid characters.";
			OutputDebugStringA(error.c_str());
		}

		std::vector<wchar_t> decodedURLWide(result);
		MultiByteToWideChar(
			CP_UTF8, 0,
			decodedURL.c_str(), -1,
			decodedURLWide.data(), decodedURLWide.size());

		std::ifstream ifs(decodedURLWide.data(), std::ios::binary);
#else
		std::ifstream ifs(decodedURL, std::ios::binary);
#endif
		if (ifs.good())
		{
			ifs.seekg(0, std::ios::end);
			size_t filesize = size_t(ifs.tellg());
			ifs.seekg(0, std::ios::beg);

			std::vector<char> buffer(filesize);
			ifs.read(&buffer[0], filesize);

			// Status *must* be set before receiving any data
			response->SetStatus(200);
			response->SetExpectedLength(filesize);

			response->ReceiveData(&buffer[0], filesize);
			response->SignalSuccess();
		}
		else
		{
			response->SignalFailure();
		}

		response->Release();
	}
};