//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license201809 for the full license information.
//

#include "stdafx.h"
#include "handle_helpers.h"
#include "event_helpers.h"

using namespace Microsoft::CognitiveServices::Speech::Impl;

SPXAPI connection_from_recognizer(SPXRECOHANDLE recognizerHandle, SPXCONNECTIONHANDLE* connectionHandle)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, connectionHandle == nullptr);
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, !recognizer_handle_is_valid(recognizerHandle));

    SPXAPI_INIT_HR_TRY(hr)
    {
        *connectionHandle = SPXHANDLE_INVALID;

        auto recoHandleTable = CSpxSharedPtrHandleTableManager::Get<ISpxRecognizer, SPXRECOHANDLE>();
        auto recognizer = (*recoHandleTable)[recognizerHandle];
        SPX_IFTRUE_THROW_HR(recognizer == nullptr, SPXERR_INVALID_HANDLE);

        auto recoForConnection = SpxQueryInterface<ISpxConnectionFromRecognizer>(recognizer);
        SPX_IFTRUE_THROW_HR(recoForConnection == nullptr, SPXERR_EXPLICIT_CONNECTION_NOT_SUPPORTED_BY_RECOGNIZER);
        auto connection = recoForConnection->GetConnection();

        auto connectionHandleTable = CSpxSharedPtrHandleTableManager::Get<ISpxConnection, SPXCONNECTIONHANDLE>();
        *connectionHandle = connectionHandleTable->TrackHandle(connection);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}


SPXAPI_(bool) connection_handle_is_valid(SPXCONNECTIONHANDLE handle)
{
    return Handle_IsValid<SPXCONNECTIONHANDLE, ISpxConnection>(handle);
}

SPXAPI connection_handle_release(SPXCONNECTIONHANDLE handle)
{
    return Handle_Close<SPXCONNECTIONHANDLE, ISpxConnection>(handle);
}

SPXAPI connection_connected_set_callback(SPXCONNECTIONHANDLE connection, CONNECTION_CALLBACK_FUNC callback, void* context)
{
    return connection_set_event_callback(&ISpxRecognizerEvents::Connected, connection, callback, context);
}

SPXAPI connection_disconnected_set_callback(SPXCONNECTIONHANDLE connection, CONNECTION_CALLBACK_FUNC callback, void* context)
{
    return connection_set_event_callback(&ISpxRecognizerEvents::Disconnected, connection, callback, context);
}

SPXAPI connection_message_received_set_callback(SPXCONNECTIONHANDLE connection, CONNECTION_CALLBACK_FUNC callback, void* context)
{
    return connection_message_set_event_callback(&ISpxRecognizerEvents::ConnectionMessageReceived, connection, callback, context);
}

SPXAPI connection_open(SPXCONNECTIONHANDLE handle, bool forContinuousRecognition)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_HANDLE, !connection_handle_is_valid(handle));

    SPXAPI_INIT_HR_TRY(hr)
    {
        auto handleTable = CSpxSharedPtrHandleTableManager::Get<ISpxConnection, SPXCONNECTIONHANDLE>();
        auto connection = (*handleTable)[handle];
        SPX_IFTRUE_THROW_HR(connection == nullptr, SPXERR_INVALID_HANDLE);
        connection->Open(forContinuousRecognition);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI connection_close(SPXCONNECTIONHANDLE handle)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, !connection_handle_is_valid(handle));

    SPXAPI_INIT_HR_TRY(hr)
    {
        auto handleTable = CSpxSharedPtrHandleTableManager::Get<ISpxConnection, SPXCONNECTIONHANDLE>();
        auto connection = (*handleTable)[handle];
        SPX_IFTRUE_THROW_HR(connection == nullptr, SPXERR_INVALID_HANDLE);
        connection->Close();
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI connection_set_message_property(SPXCONNECTIONHANDLE handle, const char* path, const char* name, const char* value)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, handle == nullptr);
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, name == nullptr || !(*name));
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, name == nullptr || !(*path));
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, value == nullptr || !(*value));

    SPXAPI_INIT_HR_TRY(hr)
    {
        auto handleTable = CSpxSharedPtrHandleTableManager::Get<ISpxConnection, SPXCONNECTIONHANDLE>();
        auto connection = (*handleTable)[handle];
        SPX_IFTRUE_THROW_HR(connection == nullptr, SPXERR_INVALID_HANDLE);

        auto setter = SpxQueryInterface<ISpxMessageParamFromUser>(connection);
        SPX_IFTRUE_THROW_HR(setter == nullptr, SPXERR_INVALID_ARG);

        setter->SetParameter(path, name, value);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI connection_send_message(SPXCONNECTIONHANDLE handle, const char* path, const char* payload)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, handle == nullptr);
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, payload == nullptr || !(*payload));
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, path == nullptr || !(*path));

    SPXAPI_INIT_HR_TRY(hr)
    {
        auto handleTable = CSpxSharedPtrHandleTableManager::Get<ISpxConnection, SPXCONNECTIONHANDLE>();
        auto connection = (*handleTable)[handle];

        auto setter = SpxQueryInterface<ISpxMessageParamFromUser>(connection);
        SPX_IFTRUE_THROW_HR(setter == nullptr, SPXERR_INVALID_ARG);

        setter->SendNetworkMessage(path, payload);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI connection_send_message_data(SPXCONNECTIONHANDLE handle, const char* path, uint8_t* data, uint32_t size)
{
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, handle == nullptr);
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, path == nullptr);
    SPX_RETURN_HR_IF(SPXERR_INVALID_ARG, data == nullptr);

    SPXAPI_INIT_HR_TRY(hr)
    {
        auto connection = CSpxSharedPtrHandleTableManager::GetPtr<ISpxConnection, SPXCONNECTIONHANDLE>(handle);

        auto setter = SpxQueryInterface<ISpxMessageParamFromUser>(connection);
        SPX_IFTRUE_THROW_HR(setter == nullptr, SPXERR_INVALID_ARG);

        std::vector<uint8_t> payload(data, data + size);
        setter->SendNetworkMessage(path, std::move(payload));
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI_(bool) connection_message_received_event_handle_is_valid(SPXCONNECTIONMESSAGEHANDLE handle)
{
    return Handle_IsValid<SPXEVENTHANDLE, ISpxConnectionMessageEventArgs>(handle);
}

SPXAPI connection_message_received_event_handle_release(SPXEVENTHANDLE hevent)
{
    return Handle_Close<SPXEVENTHANDLE, ISpxConnectionMessageEventArgs>(hevent);
}

SPXAPI connection_message_received_event_get_message(SPXEVENTHANDLE event, SPXCONNECTIONMESSAGEHANDLE* hcm)
{
    SPXAPI_INIT_HR_TRY(hr)
    {
        *hcm = SPXHANDLE_INVALID;

        auto connectionEventArgs = CSpxSharedPtrHandleTableManager::GetPtr<ISpxConnectionMessageEventArgs, SPXEVENTHANDLE>(event);
        auto message = connectionEventArgs->GetMessage();

        *hcm = CSpxSharedPtrHandleTableManager::TrackHandle<ISpxConnectionMessage, SPXCONNECTIONMESSAGEHANDLE>(message);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI_(bool) connection_message_handle_is_valid(SPXCONNECTIONMESSAGEHANDLE handle)
{
    return Handle_IsValid<SPXCONNECTIONMESSAGEHANDLE, ISpxConnectionMessage>(handle);
}

SPXAPI connection_message_handle_release(SPXCONNECTIONMESSAGEHANDLE handle)
{
    return Handle_Close<SPXCONNECTIONMESSAGEHANDLE, ISpxConnectionMessage>(handle);
}

SPXAPI connection_message_get_property_bag(SPXCONNECTIONMESSAGEHANDLE hcm, SPXPROPERTYBAGHANDLE* hpropbag)
{
    SPXAPI_INIT_HR_TRY(hr)
    {
        *hpropbag = SPXHANDLE_INVALID;

        auto message = CSpxSharedPtrHandleTableManager::GetPtr<ISpxConnectionMessage, SPXCONNECTIONMESSAGEHANDLE>(hcm);
        auto namedProperties = SpxQueryInterface<ISpxNamedProperties>(message);

        *hpropbag = CSpxSharedPtrHandleTableManager::TrackHandle<ISpxNamedProperties, SPXCONNECTIONMESSAGEHANDLE>(namedProperties);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI connection_message_get_data(SPXCONNECTIONMESSAGEHANDLE hcm, uint8_t* data, uint32_t size)
{
    SPXAPI_INIT_HR_TRY(hr)
    {
        auto message = CSpxSharedPtrHandleTableManager::GetPtr<ISpxConnectionMessage, SPXCONNECTIONMESSAGEHANDLE>(hcm);

        auto buffer = message->GetBuffer();
        auto bufferSize = message->GetBufferSize();
        SPX_IFTRUE_THROW_HR(size > bufferSize, SPXERR_OUT_OF_RANGE);

        memcpy(data, buffer, size);
    }
    SPXAPI_CATCH_AND_RETURN_HR(hr);
}

SPXAPI_(uint32_t) connection_message_get_data_size(SPXCONNECTIONMESSAGEHANDLE hcm)
{
    SPXAPI_INIT_HR_TRY(hr)
    {
        auto message = CSpxSharedPtrHandleTableManager::GetPtr<ISpxConnectionMessage, SPXCONNECTIONMESSAGEHANDLE>(hcm);
        return message->GetBufferSize();
    }
    SPXAPI_CATCH_AND_RETURN(hr, 0);
}