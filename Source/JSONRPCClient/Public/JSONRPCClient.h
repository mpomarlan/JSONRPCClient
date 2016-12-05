// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

#include <string>
#include <vector>
#include <map>

#include <Http.h>
#include <Json.h>

typedef struct
{
	std::string topicName;
	std::map<std::string, std::string> params;
}MessageData;

class JSONRPCClient
{
	int currentId;

	std::string URL;

public:

	void setURL(std::string const& url)
	{
		currentId = 0;
		URL = url;
	}

	void SendRPC(std::string const& method, std::map<std::string, std::string> const& params, std::map<std::string, std::string> & result)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> JsonParams = MakeShareable(new FJsonObject());

		for (std::map<std::string, std::string>::const_iterator it = params.begin();
			it != params.end(); it++)
			JsonParams->SetStringField(it->first.c_str(), it->second.c_str());

		JsonObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
		JsonObject->SetStringField(TEXT("method"), method.c_str());
		JsonObject->SetStringField(TEXT("id"), *FString::Printf(TEXT("%d"), currentId)); currentId++;
		JsonObject->SetObjectField(TEXT("params"), JsonParams);
		FString OutputString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		//Request->OnProcessRequestComplete().BindUObject(this, &JSONClient::OnResponseReceived);
		Request->SetURL(URL.c_str());
		Request->SetVerb("POST");
		Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
		Request->SetHeader("Content-Type", "application/json");
		Request->SetContentAsString(OutputString);
		Request->ProcessRequest();
                // TODO: play a bit with signals to get the output from the request.
                result.clear();
	}

        void SendRPC(std::string const& method, std::map<std::string, std::string> const& params)
        {
            std::map<std::string, std::string> result;
            SendRPC(method, params, result);
        }

	void SendRPC(std::string const& method, std::vector<MessageData> const& params, std::vector<MessageData> & response)
	{
		unsigned int maxK = params.size();
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TArray<TSharedPtr<FJsonValue>> JsonMsgArray;
		TSharedPtr<FJsonObject> JsonMsgArrayObject = MakeShareable(new FJsonObject());

		JsonMsgArray.Reserve(maxK);
		for(unsigned int k = 0; k < maxK; k++)
		{
			TSharedPtr<FJsonObject> JsonMsgParams = MakeShareable(new FJsonObject());
			TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());

			for (std::map<std::string, std::string>::const_iterator it = params[k].params.begin();
				it != params[k].params.end(); it++)
				JsonMsgParams->SetStringField(it->first.c_str(), it->second.c_str());

			JsonMsg->SetStringField(TEXT("topic"), params[k].topicName.c_str());
			JsonMsg->SetObjectField(TEXT("params"), JsonMsgParams);
			TSharedRef< FJsonValueObject > JsonValue = MakeShareable( new FJsonValueObject( JsonMsg) );
			JsonMsgArray.Add(JsonValue);
		}

		JsonObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
		JsonObject->SetStringField(TEXT("method"), method.c_str());
		JsonObject->SetStringField(TEXT("id"), *FString::Printf(TEXT("%d"), currentId)); currentId++;
		JsonMsgArrayObject->SetArrayField(TEXT("params"), JsonMsgArray);
		JsonObject->SetObjectField(TEXT("params"), JsonMsgArrayObject);

		FString OutputString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &JSONRPCClient::OnResponseReceived);
		Request->SetURL(URL.c_str());
		Request->SetVerb("POST");
		Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
		Request->SetHeader("Content-Type", "application/json");
		Request->SetContentAsString(OutputString);
		Request->ProcessRequest();

                // TODO: play a bit with signals to get the output from the request.
                response.clear();
	}

        void SendRPC(std::string const& method, std::vector<MessageData> const& params)
        {
            std::vector<MessageData> response;
            SendRPC(method, params, response);
        }

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		//UE_LOG(LogTemp, Error, TEXT("Yay, have response %s %d."), *(Response->GetContentType()), bWasSuccessful);
		//if (bWasSuccessful && Response->GetContentType() == "application/json")
		if (bWasSuccessful && Response->GetContentType() == "text/plain; charset=utf-8")
		{
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
			TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
			FJsonSerializer::Deserialize(JsonReader, JsonObject);
			TSharedPtr<FJsonObject> JsonMessagesObject = JsonObject->GetObjectField(TEXT("result"))->GetObjectField(TEXT("messages"));

			FString DbgString;
			TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&DbgString);
			FJsonSerializer::Serialize(JsonMessagesObject.ToSharedRef(), JsonWriter);
			UE_LOG(LogTemp, Error, TEXT("Yay, have response %s."), *(DbgString));

			//SomeVariable = JsonObject->GetStringField("some_response_field");
		}
		else
		{
			// Handle error here
		}
	}

};

class FJSONRPCClientModule : public IModuleInterface
{
	
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
