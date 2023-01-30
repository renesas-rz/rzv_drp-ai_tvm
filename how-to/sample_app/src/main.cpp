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
* File Name    : main.cpp
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
#include "ws_server.h"
#include "recognize_proc.h"
#include "command/change_model.h"
#include "cereal/cereal.hpp"

static RecognizeProc* g_rc;

/**
 * @brief command_rsv
 * @details on command receive
 * @param snd sender server
 * @param obj received string
 */
void command_rsv(WebsocketServer* snd, string obj)
{
    std::cout << "command received ! " << obj << std::endl;

    ChangeModel chg;
    if (chg.Deserizlie(obj) == false) {
        std::cout << "Faile to parse command... " << obj << std::endl;
        return;
    }

    std::cout << "modelchange! " << chg.model << std::endl;

    g_rc->stop_recognize();
    g_rc->switch_model(chg.model);
}

/**
 * @brief on_disconnect
 * @details stop recognize and finish.
 * @param snd sender server
 */
void on_disconnect(WebsocketServer* snd)
{
    std::cout << "***** Connection has lost *****" << std::endl;
    g_rc->stop_recognize();
    std::cout << "***** Stop Recognize. *****" << std::endl;
    g_rc->finish_recognize_thread();
}

int32_t main(int32_t argc, char* argv[])
{
    printf("################################################\n");
    printf("#    RZ/V2MA DRP-AI TVM Sample Application     #\n");
    printf("################################################\n");


    /* start ws */
    WebsocketServer ws;
    /* Start Websocket server */
    ws.start_server("192.168.1.11", "3000", "ws");
    ws.wait_connection();
    ws._commandReceived = &command_rsv;
    ws._diconnected = &on_disconnect;

    g_rc = new RecognizeProc(shared_ptr<WebsocketServer>(&ws));
    g_rc->start_recognize();

    ws.stop_server();

}
