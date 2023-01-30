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
* File Name    : ws_server.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/
/*****************************************
* Includes
******************************************/
#include "util/string_formatter.h"
#include "ws_server.h"

/**
 * @brief Start websocket server
 * @details start
 * @param IP server IP (board IP) specify by IPV4
 * @param port server port
 * @param serverName server name. e.g.) URL= ws://IP:port/servername/
 */
void WebsocketServer::start_server(string IP, string port, string serverName)
{
    std::cout << "Start Server " << format("ws://%s:%s/%s/", IP.c_str(), port.c_str(), serverName.c_str()) << std::endl;

    _server = WebSocket(WebSocket::Mode::SERVER);
    _server.bind(format("ws://%s:%s/%s/", IP.c_str(), port.c_str(), serverName.c_str()));
    _server.listen(1);
}

/**
 * @brief stop server
 * @details stop message receive loop.
 */
void WebsocketServer::stop_server()
{
    int32_t ret_status = 0;
    blRunThread = false;
    ret_status = wait_join(&message_receive_thread, MSG_RSV_THREAD_TIMEOUT);
    if (0 != ret_status)
    {
        fprintf(stderr, "[ERROR] Failed to exit message Thread on time.[%d]\n", ret_status);
    }
}
/**
 * @brief wait_connection
 * @details Wait client connection. It can only 1 client.
 */
void WebsocketServer::wait_connection()
{
    WebSocket ws = _server.accept();  /*blocking, return new WebSocket object*/
    _client = move(ws);

    _client.recv_req();
    _client.send_res();

    pthread_create(&message_receive_thread, NULL, &message_workerWrapper, this);

    std::cout << "Client Connected." << std::endl;

}

/**
 * @brief message_workerWrapper
 * @details
 * @param object pointer to itself
 * @return void*
 */
void* WebsocketServer::message_workerWrapper(void* object)
{
    message_worker(reinterpret_cast<WebsocketServer*>(object));
    return NULL;
}

/**
 * @brief message_worker
 * @details Websocket message loop.
 * @param obj pointer to itself
 */
void WebsocketServer::message_worker(WebsocketServer* obj)
{
    obj->blRunThread = true;
    Timespec timeout(10000); /* msec */
    while (obj->blRunThread)
    {
        if (obj->_client.sfd_ref() <= 0)
        {
            std::cout << "websocket disconnected" << std::endl;
            std::cout << "websocket connection has lost. message receive will be abort." << std::endl;
            if (obj->_diconnected != NULL)
            {
                obj->_diconnected(obj);
            }
            break;
        }
        try
        {
            std::pair<std::string, int32_t> A = obj->_client.recv_msg_txt(timeout);
            std::cout << A.first << std::endl;
            if (A.first.size() > 0)
            {
                std::cout << "Websocket command received " << std::endl;
                if (obj->_commandReceived != NULL)
                {
                    obj->_commandReceived(obj, A.first);
                }
            }
        }
        catch (SystemErrorException e)
        {
            if (e.code() != (int32_t)lwsock::LwsockErrc::TIMED_OUT)
            {
                cout << "System error occcord. [" << e.code() << "]" << endl;
            }
        }
        catch (LwsockException e)
        {
            cout << "Lwsock error occcord. [" << e.code() << "]" << endl;
        }
    }
    cout << "<<<<<<<<<<<<<<<<<<<<< Message Thread Terminated >>>>>>>>>>>>>>>>>>" << endl;

    pthread_exit(NULL);
}

/**
 * @brief send_command
 * @details Send websocket data by sting.
 * @param command data string
 * @return true success
 * @return false fail
 */
bool WebsocketServer::send_command(std::string command)
{
    if (_client.sfd_ref() <= 0) return false;
    return _client.send_msg_txt(command) > 0;
}

/**
 * @brief wait_join
 * @details wait for thread
 * @param p_join_thread target thread.
 * @param join_time timeout time[sec]
 * @return int8_t success:0
 */
int8_t WebsocketServer::wait_join(pthread_t* p_join_thread, uint32_t join_time)
{
    int8_t ret_err;
    struct timespec join_timeout;
    ret_err = clock_gettime(CLOCK_REALTIME, &join_timeout);
    if (0 == ret_err)
    {
        join_timeout.tv_sec += join_time;
        ret_err = pthread_timedjoin_np(*p_join_thread, NULL, &join_timeout);
    }
    return ret_err;
}
