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

#include "deviceview.h"

#include "device.h"
#include "graph.h"
#include "setting.h"
#include "settingstore.h"
#include "stringutils.h"
#include "window.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>      // 新增的头文件
#include <string>   // 新增的头文件

using namespace std;

DeviceView::DeviceView(Device* device)
    : m_deviceNumber(0), m_totalNumberOfDevices(0), m_device(device)
{
}

DeviceView::~DeviceView()
{
}

void DeviceView::update()
{
    if(m_device->exists())
    {
        const Statistics& statistics = m_device->getStatistics();

        m_deviceGraphIn.update(statistics.getDataInPerSecond());
        m_deviceGraphOut.update(statistics.getDataOutPerSecond());
    }
    else
    {
        m_deviceGraphIn.resetTrafficData();
        m_deviceGraphOut.resetTrafficData();
    }
}

// print the device's data
void DeviceView::print(Window& window)
{
    int width = window.getWidth();
    int height = window.getHeight();

    // print header
    if(height > 2)
    {
        string deviceName = m_device->getName();
        string ipv4 = m_device->getIpV4Address();

        if(m_device->exists())
        {
            if(!ipv4.empty())
                window.print() << "Device " << deviceName << " [" << ipv4 << "] (" << (m_deviceNumber + 1) << "/" << m_totalNumberOfDevices << "):" << endl;
            else
                window.print() << "Device " << deviceName << " (" << (m_deviceNumber + 1) << "/" << m_totalNumberOfDevices << "):" << endl;
            window.print() << string(width, '=');
        }
        else
        {
            // if device does not exist print warning message
            window.print() << "Device " << deviceName << " (" << (m_deviceNumber + 1) << "/" << m_totalNumberOfDevices << "): does not exist" << endl;
            window.print() << string(width, '=') << endl;

            // and exit
            return;
        }
    }
    if(width < 25 || height < 8)
    {
        window.print() << "Please enlarge console for viewing device information." << endl;
        return;
    }

    // format statistics
    vector<string> statLinesIn;
    vector<string> statLinesOut;

    generateStatisticsIn(statLinesIn);
    generateStatisticsOut(statLinesOut);

    size_t statLineInMaxLength = max_element(statLinesIn.begin(), statLinesIn.end(), sizeLess())->size();
    size_t statLineOutMaxLength = max_element(statLinesOut.begin(), statLinesOut.end(), sizeLess())->size();
    int statLineMaxLength = statLineInMaxLength > statLineOutMaxLength ? statLineInMaxLength : statLineOutMaxLength;

    // if graphs should be hidden ...
    if(SettingStore::get("MultipleDevices"))
    {
        window.print() << "Incoming:";
        window.print(width / 2) << "Outgoing:" << endl;

        int statusY = window.getY();

        printStatistics(window, statLinesIn, 0, statusY);
        printStatistics(window, statLinesOut, width / 2, statusY);

        window.print() << endl;
    }
    // ... or not
    else
    {
        // calculate layout
        int lines = height - window.getY();
        int linesForIn = (lines + 1) / 2;
        int linesForOut = lines - linesForIn;
        int dirInY = window.getY();
        int dirOutY = dirInY + linesForIn;

//        int statisticsX = width - statLineMaxLength - 1;
//        statisticsX -= statisticsX % 5;
//
        // 计算统计信息区域的起始 X 坐标
int statisticsX = width * 0.7; // 统计信息区域占用屏幕宽度的40%
if (statisticsX + statLineMaxLength >= width)
{
    statisticsX = width - statLineMaxLength - 1;
}

        if(linesForOut <= 5)
        {
            linesForIn = lines;
            linesForOut = 0;
            dirOutY = height;
        }

        // calculate deflection of graphs
        unsigned long long maxDeflectionIn = (unsigned long long) SettingStore::get("BarMaxIn") * 1024 / 8;
        unsigned long long maxDeflectionOut = (unsigned long long) SettingStore::get("BarMaxOut") * 1024 / 8;

        if(maxDeflectionIn < 1)
            maxDeflectionIn = roundUpMaxDeflection(m_deviceGraphIn.calcMaxDeflection());
        if(maxDeflectionOut < 1)
            maxDeflectionOut = roundUpMaxDeflection(m_deviceGraphOut.calcMaxDeflection());

        // print incoming data
        if(linesForIn > 5)
        {
//            window.print(0, dirInY) << "Incoming (100% @ " << formatTrafficValue(maxDeflectionIn, 0) << "):" << endl;
            window.print(0, dirInY) << "Incoming (100% @ " << formatTrafficValue(maxDeflectionIn) << "):" << endl;

            if(statisticsX > 1)
            {
                m_deviceGraphIn.setNumOfBars(statisticsX - 1);
                m_deviceGraphIn.setHeightOfBars(linesForIn - 1);
                m_deviceGraphIn.setMaxDeflection(maxDeflectionIn);
                m_deviceGraphIn.print(window, 0, dirInY + 1);
            }

            if(width > statLineMaxLength)
                printStatistics(window, statLinesIn, statisticsX, dirInY + linesForIn - 5);
        }


        // print outgoing data
        if(linesForOut > 5)
        {
  //          window.print(0, dirOutY) << "Outgoing (100% @ " << formatTrafficValue(maxDeflectionOut, 0) << "):" << endl;
window.print(0, dirOutY) << "Outgoing (100% @ " << formatTrafficValue(maxDeflectionOut) << "):" << endl;
            if(statisticsX > 1)
            {
                m_deviceGraphOut.setNumOfBars(statisticsX - 1);
                m_deviceGraphOut.setHeightOfBars(linesForOut - 1);
                m_deviceGraphOut.setMaxDeflection(maxDeflectionOut);
                m_deviceGraphOut.print(window, 0, dirOutY + 1);
            }

            if(width > statLineMaxLength)
                printStatistics(window, statLinesOut, statisticsX, dirOutY + linesForOut - 5);
        }
    }
}

// set the number identifying the device (for display only)
void DeviceView::setDeviceNumber(int deviceNumber)
{
    m_deviceNumber = deviceNumber;
}

// set the total number of shown devices (for display only)
void DeviceView::setTotalNumberOfDevices(int totalNumberOfDevices)
{
    m_totalNumberOfDevices = totalNumberOfDevices;
}

unsigned long long DeviceView::roundUpMaxDeflection(unsigned long long value)
{
    unsigned long long rounded = 2 * 1024; // 2 kByte/s
    while (rounded < value)
    {
        if((rounded << 1) < rounded)
            return value;

        rounded <<= 1;
    }

    return rounded;
}


string DeviceView::formatTrafficValue(unsigned long value)
{
    // 将字节数转换为比特数
    unsigned long long bitValue = static_cast<unsigned long long>(value) * 8;

    // 将比特数转换为兆比特每秒（Mb/s）
    unsigned long long megabitValue = bitValue / 1000000; // 使用十进制的 1,000,000

    // 将兆比特数转换为字符串
    stringstream ss;
    ss << megabitValue;

    return ss.str();
}


void DeviceView::generateStatisticsIn(vector<string>& statisticLines)
{
    const Statistics& statistics = m_device->getStatistics();

    statisticLines.push_back("Cur: " + formatTrafficValue(statistics.getDataInPerSecond()));
    // 如果需要，可以删除或注释掉其他统计数据行
    statisticLines.push_back("Avg: " + formatTrafficValue(statistics.getDataInAverage())+ " Mb/s" ) ;
    statisticLines.push_back("Min: " + formatTrafficValue(statistics.getDataInMin())+ " Mb/s");
    statisticLines.push_back("Max: " + formatTrafficValue(statistics.getDataInMax())+ " Mb/s");
    statisticLines.push_back("Ttl: " + formatDataValue(statistics.getDataInTotal(),2));

}

void DeviceView::generateStatisticsOut(vector<string>& statisticLines)
{
    const Statistics& statistics = m_device->getStatistics();

    statisticLines.push_back("Cur: " + formatTrafficValue(statistics.getDataOutPerSecond())+ " Mb/s");
    statisticLines.push_back("Avg: " + formatTrafficValue(statistics.getDataOutAverage())+ " Mb/s");
    statisticLines.push_back("Min: " + formatTrafficValue(statistics.getDataOutMin())+ " Mb/s");
    statisticLines.push_back("Max: " + formatTrafficValue(statistics.getDataOutMax())+ " Mb/s");
    statisticLines.push_back("Ttl: " + formatDataValue(statistics.getDataOutTotal(),2));

}



string DeviceView::formatDataValue(unsigned long long value, int precision)
{
    Statistics::dataUnit dataFormat = (Statistics::dataUnit) ((int) SettingStore::get("DataFormat"));

    string unitString = Statistics::getUnitString(dataFormat, value);
    float unitFactor = Statistics::getUnitFactor(dataFormat, value);

    ostringstream oss;
    oss << fixed << setprecision(precision) << ((float) value / unitFactor) << " " << unitString << endl;

    return oss.str();
}


void DeviceView::printStatistics(Window& window, const vector<string>& statisticLines, int x, int y)
{
    for (vector<string>::const_iterator itLine = statisticLines.begin(); itLine != statisticLines.end(); ++itLine)
    {
        if (itLine->substr(0, 4) == "Cur:")
        {
            // 这是 "Cur" 行，提取数值并以大号字体打印
            std::string curLine = *itLine;
            // 提取 "Cur: " 后的数值部分
            std::string curValue = curLine.substr(5); // 跳过 "Cur: "

            // 只保留前4位数字
            if (curValue.length() > 4)
            {
                curValue = curValue.substr(0, 4);
            }

            // 将数字向上移动2行
            int adjustedY = y - 6;
            if (adjustedY < 0)
                adjustedY = 0;

            // 打印大号字体的数值
            printLargeNumber(window, x, adjustedY, curValue);

            // 更新 y 坐标，确保后续内容不被覆盖
            y = adjustedY + 7; // 大号字体占用了7行
        }
        else
        {
            // 正常打印
            window.print(x, y++) << *itLine;
        }
    }
}


// 添加新的函数，用于打印大号字体的数值
void DeviceView::printLargeNumber(Window& window, int x, int y, const std::string& value)
{
    // 定义数字和字符的 ASCII 艺术表示
    static const char* largeChars[][7] = {
        // '0'
        {
            "  ___  ",
            " / _ \\ ",
            "| | | |",
            "| | | |",
            "| |_| |",
            " \\___/ ",
            "       "
        },
        // '1'
        {
            " __    ",
            "/_ |   ",
            " | |   ",
            " | |   ",
            " | |   ",
            " |_|   ",
            "       "
        },
        // '2'
        {
            "  ___  ",
            " |__ \\ ",
            "    ) |",
            "   / / ",
            "  / /_ ",
            " |____|",
            "       "
        },
        // '3'
        {
            "  ____ ",
            " |___ \\",
            "   __) |",
            "  |__ < ",
            "  ___) |",
            " |____/ ",
            "        "
        },
        // '4'
        {
            "  _  _   ",
            " | || |  ",
            " | || |_ ",
            " |__   _|",
            "    | |  ",
            "    |_|  ",
            "         "
        },
        // '5'
        {
            "  _____ ",
            " | ____|",
            " | |__  ",
            " |___ \\ ",
            "  ___) |",
            " |____/ ",
            "        "
        },
        // '6'
        {
            "   __   ",
            "  / /   ",
            " / /_   ",
            " | '_ \\ ",
            " | (_) |",
            "  \\___/ ",
            "        "
        },
        // '7'
        {
            "  ______",
            " |____  |",
            "     / / ",
            "    / /  ",
            "   / /   ",
            "  /_/    ",
            "         "
        },
        // '8'
        {
            "   ___  ",
            "  / _ \\ ",
            " | (_) |",
            "  > _ < ",
            " | (_) |",
            "  \\___/ ",
            "        "
        },
        // '9'
        {
            "   ___  ",
            "  / _ \\ ",
            " | (_) |",
            "  \\__, |",
            "    / / ",
            "   /_/  ",
            "        "
        },
        // '.'
        {
            "       ",
            "       ",
            "       ",
            "       ",
            "  ___  ",
            " (___) ",
            "       "
        },
        // 'K'
        {
            "  _  __",
            " | |/ /",
            " | ' / ",
            " |  <  ",
            " | . \\ ",
            " |_|\\_\\",
            "       "
        },
        // 'M'
        {
            "  __  __ ",
            " |  \\/  |",
            " | \\  / |",
            " | |\\/| |",
            " | |  | |",
            " |_|  |_|",
            "         "
        },
        // 'G'
        {
            "  ____ ",
            " / ___|",
            "| |  _ ",
            "| |_| |",
            " \\____|",
            "       ",
            "       "
        },
        // 'b'
        {
            " _     ",
            "| |    ",
            "| |__  ",
            "| '_ \\ ",
            "| |_) |",
            "|_.__/ ",
            "       "
        },
        // 'i'
        {
            " _ ",
            "(_)",
            " | ",
            " | ",
            " | ",
            " |_|",
            "    "
        },
        // 't'
        {
            "  _   ",
            " | |  ",
            " | |_ ",
            " | __|",
            " | |_ ",
            "  \\__|",
            "      "
        },
        // '/'
        {
            "     __",
            "    / /",
            "   / / ",
            "  / /  ",
            " / /   ",
            "/_/    ",
            "       "
        },
        // 's'
        {
            "  ____ ",
            " / ___|",
            " \\___ \\",
            "  ___) |",
            " |____/ ",
            "        ",
            "        "
        }
    };

    // 映射字符到索引
    std::map<char, int> charMap = {
        {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4},
        {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9},
        {'.', 10}, {'K', 11}, {'M', 12}, {'G', 13}, {'b', 14},
        {'i', 15}, {'t', 16}, {'/', 17}, {'s', 18}
    };

    // 对于 ASCII 艺术字体的每一行
    for (int i = 0; i < 7; ++i)
    {
        int xPos = x;
        for (size_t j = 0; j < value.length(); ++j)
        {
            char c = value[j];
            const char* line;
            if (charMap.find(c) != charMap.end())
            {
                int idx = charMap[c];
                line = largeChars[idx][i];
            }
            else
            {
                // 未定义的字符，用空格替代
                line = "       ";
            }
            window.print(xPos, y + i) << line;
            xPos += 8; // 每个字符宽度为7，加1个空格
        }
    }
}
