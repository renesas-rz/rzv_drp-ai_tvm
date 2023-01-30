/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : ws_server.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/
#ifndef _WS_SERVER_H_
#define _WS_SERVER_H_

/*****************************************
* Includes
******************************************/
#include "../include/lwsock.hpp"
#include "includes.h"

using namespace std;
using namespace lwsock;

#define MSG_RSV_THREAD_TIMEOUT (10000)

class WebsocketServer
{
public:
    WebsocketServer() {
        _commandReceived = NULL;
        _diconnected = NULL;
    }
    void start_server(string IP, string port, string serverName);
    void stop_server();
    void wait_connection();
    bool send_command(std::string command);
private:
    static void message_worker(WebsocketServer* obj);
    static void* message_workerWrapper(void* object);
    int8_t wait_join(pthread_t* p_join_thread, uint32_t join_time);


public:
    void (*_commandReceived)(WebsocketServer* sender, string rcv_data);
    void (*_diconnected)(WebsocketServer* sender);
private:

    bool blRunThread;
    pthread_t message_receive_thread;

    WebSocket _server;
    WebSocket _client;
};

#endif // !_WS_SERVER_H_
