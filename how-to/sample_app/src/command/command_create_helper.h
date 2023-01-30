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
* File Name    : command_create_helper.h
* Version      : 1.0.4
* Description  : RZ/V2MA DRP-AI TVM[*1] Sample Application for USB Camera HTTP version
*                *1 DRP-AI TVM is powered by EdgeCortix MERA(TM) Compiler Framework.
***********************************************************************************************************************/

#pragma once
#ifndef COMMAND_CREATE_HELPER_H
#define COMMAND_CREATE_HELPER_H
/*****************************************
* Includes
******************************************/

#include <iostream>
#include <sstream>
#include <memory>
#include<string>
#include<map>
#include<vector>
#include <stdio.h>
#include "../include/cereal/cereal.hpp"
#include "../include/cereal/archives/json.hpp"
#include "../include/cereal/types/string.hpp"
#include "../include/cereal/types/vector.hpp"
#include "../include/cereal/types/polymorphic.hpp"
class CommandCreateHelper
{
#define COMMAND_ID_KEY "command_name"
#define VALUE_KEY "Value"

public:

    CommandCreateHelper() {}
    virtual~CommandCreateHelper() {}

    template<class Archive>
    void save(Archive& archive) const
    {
    }

    template<class Archive>
    void load(Archive& archive)
    {
    }



    /**
     * @brief SerializeCommandBody
     * @details Serialize command class to json
     * @tparam T Command classs type
     * @param src target command
     * @return std::string Serialized json
     */
    template<class T>
    static std::string SerializeCommandBody(T& src)
    {
        std::stringstream ss;
        {
            cereal::JSONOutputArchive o_archive(ss);
            o_archive(cereal::make_nvp(COMMAND_ID_KEY, src.command_name));
            o_archive(cereal::make_nvp(VALUE_KEY, src));

        }
        return ss.str();
    }


    /**
     * @brief DeserializeCommandBody
     * @details Deserialize command class from json string
     * @tparam T Command class type
     * @param src target json
     * @return std::shared_ptr<T> serialized object
     */
    template<class T>
    static std::shared_ptr<T> DeserializeCommandBody(std::string src)
    {
        try
        {
            std::shared_ptr<T> ret = std::make_shared<T>();
            std::stringstream ss(src);
            {
                cereal::JSONInputArchive i_archive(ss);
                i_archive(cereal::make_nvp(COMMAND_ID_KEY, ret->command_name));
                i_archive(cereal::make_nvp(VALUE_KEY, *ret));
            }
            return ret;
        }
        catch (...)
        {
            std::cout << "Deserialize Error" << std::endl;
            return NULL;
        }
    }
};

#endif //COMMAND_CREATE_HELPER_H
