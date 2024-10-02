/*
 * nload
 * real time monitor for network traffic
 * Copyright (C) 2001 - 2018 by Roland Riegel <feedback@roland-riegel.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include "dataframe.h"
#include "graph.h"
#include "window.h"       // 确保包含 Window 类的头文件

#include <string>
#include <vector>
#include <map>            // 添加 map 头文件
#include <string>         // 确保包含 string 头文件

class Device;

class DeviceView
{
    public:
        explicit DeviceView(Device* device);
        ~DeviceView();

        void update();
        void print(Window& window);

        void setDeviceNumber(int deviceNumber);
        void setTotalNumberOfDevices(int totalNumberOfDevices);

    private:
        unsigned long long roundUpMaxDeflection(unsigned long long value);
            // 修改函数签名
    std::string formatTrafficValue(unsigned long value);

    std::string formatDataValue(unsigned long long value, int precision);
    void generateStatisticsIn(std::vector<std::string>& statisticLines);
    void generateStatisticsOut(std::vector<std::string>& statisticLines);
    void printStatistics(Window& window, const std::vector<std::string>& statisticLines, int x, int y);

        // 添加新的函数声明
        void printLargeNumber(Window& window, int x, int y, const std::string& value);

        int m_deviceNumber;
        int m_totalNumberOfDevices;

        Graph m_deviceGraphIn;
        Graph m_deviceGraphOut;

        Device* m_device;
};

#endif // DEVICEVIEW_H
