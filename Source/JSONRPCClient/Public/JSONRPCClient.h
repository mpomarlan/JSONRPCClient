// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

#include <string>
#include <vector>
#include <map>

#include <Http.h>
#include <Json.h>

#ifdef __linux__
#include <CoreObject.h>
#include <CoreUObject.h>
#include <UObjectBase.h>
#include <UObjectBaseUtility.h>
#include <UObject.h>
#include <UObjectArray.h>

#include <ThreadingBase.h>
#include <PThreadEvent.h>

typedef FPThreadEvent TEvent;

void initEvent(TEvent & ev)
{
	//Nothing to do here on Linux;;
}

void triggerEvent(TEvent & ev)
{
	ev.Trigger();
}

void waitEvent(TEvent & ev)
{
	ev.Wait();
}

void resetEvent(TEvent & ev)
{
	ev.Reset();
}

void destroyEvent(TEvent & ev)
{
	//Nothing to do here on Linux;;
}

#else
#include <Windows.h>
typedef void* TEvent;

void initEvent(TEvent & ev)
{
	ev = CreateEvent(
		NULL,               // default security attributes
		true,               // manual-reset event
		false,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
	);
}

void triggerEvent(TEvent & ev)
{
	SetEvent(ev);
}

void waitEvent(TEvent & ev)
{
	WaitForSingleObject(
		ev, // event handle
		INFINITE);    // indefinite wait
}

void resetEvent(TEvent & ev)
{
	ResetEvent(ev);
}

void destroyEvent(TEvent & ev)
{
	CloseHandle(ev);
}

#endif


typedef struct
{
	std::string topicName;
	std::map<std::string, std::string> params;
}MessageData;

class JSONRPCCLIENT_API JSONRPCClient
{
	int currentId;

	std::string URL;

	TEvent reqDone;

	bool allOk;
	std::vector<MessageData> response;

public:

	JSONRPCClient();

	~JSONRPCClient();

	bool isLastRequestOk(void);
	std::vector<MessageData>const& getResponse(void);

	void setURL(std::string const& url);

	void sendRPC(std::string const& method, std::map<std::string, std::string> const& params);

	void sendRPC(std::string const& method, std::vector<MessageData> const& params);

	void sendRPC(std::string const& method, std::vector<MessageData> const& params, std::vector<MessageData> & responseP);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

class FJSONRPCClientModule : public IModuleInterface
{

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
