// m3u8_2_mp4.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "m3u8_2_mp4.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 生成bat文件
const char* BAT_OUTPUT_DIRECTORY = "-output";
const char* M3U8_OUTPUT_DIRECTORY = "\\m3u8";
const char* MP4_OUTPUT_DIRECTORY = "\\mp4";
std::string current_path = "";
bool run();
bool get_current_dir_all_m3u8_file(std::vector<std::string>& m3u8_list);
bool create_output_directory();
bool modify_current_path_in_m3u8(std::vector<std::string>& m3u8_list);
bool create_bat_for_m3u8_2_mp4(std::vector<std::string>& m3u8_list);
void GetCategoryFileList(std::vector<std::string>& vecCategoryFileList, const CString& cstrPath, const CString& cstrExt);
std::string& replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value);

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            do
            {
                do
                {
                    // enter m3u8 path( no '/' or '\')
                    char buf_path[512] = { 0 };
                    printf("Input the m3u8 path: ");
                    std::cin >> buf_path;
                    if (0 == strcmp("", buf_path)) {
                        printf("Error: path(%s) is empty!\n", buf_path);
                        continue;
                    }
                    else {
                        current_path = buf_path;
                        break;
                    }
                } while (true);

                if (!run()) {
                    printf("Error: fail to create bat.\n");
                    break;
                }
                else {
                    printf("Info: create bat file success.\n");
                    break;
                }
            } while (0);
            char buf = '\0';
            printf("Enter a character to end.\n");
            std::cin >> buf;
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }
    return nRetCode;
}


bool run()
{
    if (current_path.empty()) return false;

    // 1 获取当前目录下所有m3u8文件名
    std::vector<std::string> m3u8_list;
    if (!get_current_dir_all_m3u8_file(m3u8_list)) {
        printf("Error: fail to get current dir(%s) all m3u8 file.\n", current_path.c_str());
        return false;
    }
    // 2 创建目录
    if (!create_output_directory()) {
        printf("Error: fail to create output directory(%s%s).\n", current_path.c_str(), BAT_OUTPUT_DIRECTORY);
        return false;
    }
    // 3 修改m3u8文件中路径为当前文件所在路径
    if (!modify_current_path_in_m3u8(m3u8_list)) {
        printf("Error: fail to modify current path in m3u8.\n");
        return false;
    }
    // 4 生成bat文件
    if (!create_bat_for_m3u8_2_mp4(m3u8_list)) {
        printf("Error: fail to create bat file for m3u8 trans mp4.\n");
        return false;
    }
    return true;
}

bool get_current_dir_all_m3u8_file(std::vector<std::string>& m3u8_list)
{
    m3u8_list.clear();
    GetCategoryFileList(m3u8_list, current_path.c_str(), "m3u8");
    return (m3u8_list.empty()) ? false : true;
}

bool create_output_directory()
{
    bool ret = true;
    WIN32_FIND_DATA findData = { 0 };
    HANDLE handle = INVALID_HANDLE_VALUE;
    // create root output dir
    std::string output_dir = current_path + BAT_OUTPUT_DIRECTORY;
    handle = ::FindFirstFile(output_dir.c_str(), &findData);
    if (!(INVALID_HANDLE_VALUE != handle
        && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
    {
        if (ret = ::CreateDirectory(output_dir.c_str(), NULL)) {
            printf("Error: fail to create root output dir (%s).\n", output_dir.c_str());
        }
    }
    // create m3u8 output dir
    memset(&findData, 0, sizeof(findData));
    std::string m3u8_output_dir = output_dir + M3U8_OUTPUT_DIRECTORY;
    handle = ::FindFirstFile(m3u8_output_dir.c_str(), &findData);
    if (!(INVALID_HANDLE_VALUE != handle
        && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
    {
        if (ret = ::CreateDirectory(m3u8_output_dir.c_str(), NULL)) {
            printf("Error: fail to create m3u8 output dir (%s).\n", m3u8_output_dir.c_str());
        }
    }
    // create mp4 output dir
    memset(&findData, 0, sizeof(findData));
    std::string mp4_output_dir = output_dir + MP4_OUTPUT_DIRECTORY;
    handle = ::FindFirstFile(mp4_output_dir.c_str(), &findData);
    if (!(INVALID_HANDLE_VALUE != handle
        && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
    {
        if (ret = ::CreateDirectory(mp4_output_dir.c_str(), NULL)) {
            printf("Error: fail to create mp4 output dir (%s).\n", mp4_output_dir.c_str());
        }
    }
    return ret;
}

bool modify_current_path_in_m3u8(std::vector<std::string>& m3u8_list)
{
    bool ret = true;
    for (auto& m3u8_file : m3u8_list) {
        printf("The m3u8 file is %s\n", m3u8_file.c_str());
        std::ifstream input;
        input.open(m3u8_file.c_str());
        if (!input.is_open()) {
            printf("Error: fail to open file(%s).\n", m3u8_file.c_str());
            continue;
        }

        do
        {
            bool is_first_line = false;
            std::string first_line;
            std::string tmp;
            while (!input.eof()) {
                char buf[1024] = { 0 };
                input.getline(buf, sizeof(buf));
                tmp += buf;
                tmp += "\n";
                if ('/' == buf[0] && !is_first_line) {
                    is_first_line = !is_first_line;
                    first_line = buf;
                }
            }
            size_t index = std::string::npos;
            if (std::string::npos == (index = first_line.find_last_of('/'))) {
                printf("Error: no find first last '/' in (%s) \n", first_line.c_str());
                ret = false;
                break;
            }
            if (std::string::npos == (index = first_line.find_last_of('/', index - 1))) {
                printf("Error: no find second last '/' in (%s) \n", first_line.c_str());
                ret = false;
                break;
            }
            std::string old_str = first_line.substr(0, index);
            tmp = replace_all_distinct(tmp, old_str, current_path);

            if (std::string::npos == (index = m3u8_file.find_last_of('.'))) {
                printf("Error: no find first last '.' in (%s) \n", m3u8_file.c_str());
                ret = false;
                break;
            }
            std::string new_m3u8_file_path = m3u8_file;
            new_m3u8_file_path.insert(index, "_tmp");
            if (std::string::npos == (index = new_m3u8_file_path.find_last_of('\\'))) {
                printf("Error: no find first last '\\' in (%s) \n", new_m3u8_file_path.c_str());
                ret = false;
                break;
            }
            std::string m3u8_tmp = std::string(BAT_OUTPUT_DIRECTORY) + std::string(M3U8_OUTPUT_DIRECTORY);
            new_m3u8_file_path.insert(index, m3u8_tmp);
            std::ofstream output;
            output.open(new_m3u8_file_path.c_str());
            if (!output.is_open()) {
                printf("Error: fail to open file(%s). \n", new_m3u8_file_path.c_str());
                ret = false;
                break;
            }
            output.write(tmp.c_str(), tmp.size());
            output.close();
        } while (0);

        input.close();
    }
    return ret;
}

bool create_bat_for_m3u8_2_mp4(std::vector<std::string>& m3u8_list)
{
    std::string template_path = "ffmpeg -allowed_extensions ALL -protocol_whitelist \"file,http,crypto,tcp\" -i %s -c copy %s\n";
    std::string bat_path = current_path + BAT_OUTPUT_DIRECTORY + "\\trans_m3u8_to_mp4.bat";

    ofstream output;
    output.open(bat_path.c_str());
    if (!output.is_open()) {
        printf("Error: fail to open file(%s).\n", bat_path.c_str());
        return false;
    }
    bool ret = true;
    for (auto& m3u8_file : m3u8_list) {
        size_t index = std::string::npos;
        if (std::string::npos == (index = m3u8_file.find_last_of('.'))) {
            printf("Error: no find first last '.' in (%s) \n", m3u8_file.c_str());
            ret = false;
            break;
        }
        std::string new_m3u8_path = m3u8_file;
        new_m3u8_path.insert(index, "_tmp");
        std::string m3u8_tmp = std::string(BAT_OUTPUT_DIRECTORY) + std::string(M3U8_OUTPUT_DIRECTORY);
        new_m3u8_path.insert(new_m3u8_path.find_last_of('\\'), m3u8_tmp);
        std::string new_mp4_path = m3u8_file.substr(0, index) + ".mp4";
        std::string mp4_tmp = std::string(BAT_OUTPUT_DIRECTORY) + std::string(MP4_OUTPUT_DIRECTORY);
        new_mp4_path.insert(new_mp4_path.find_last_of('\\'), mp4_tmp);
        char buf[1024] = { 0 };
        sprintf_s(buf, template_path.c_str(), new_m3u8_path.c_str(), new_mp4_path.c_str());
        output.write(buf, strlen(buf));
    }
    output.close();
    return ret;
}


void GetCategoryFileList(std::vector<std::string>& vecCategoryFileList,
    const CString& cstrPath, const CString& cstrExt)
{
    if (0 == cstrPath.GetLength())
        return;

    CFileFind fileFinder;
    CString filePath;
    filePath.Format(_T("%s\\*.*"), cstrPath);

    BOOL bFinished = fileFinder.FindFile(filePath);
    while (bFinished)
    {
        bFinished = fileFinder.FindNextFile();
        if (fileFinder.IsDirectory() && !fileFinder.IsDots())
        {
            GetCategoryFileList(vecCategoryFileList, fileFinder.GetFilePath(), cstrExt);
        }
        else
        {
            CString fileName = fileFinder.GetFileName();
            int dotPos = fileName.ReverseFind('.');
            CString fileExt = fileName.Right(fileName.GetLength() - dotPos - 1);
            if (fileExt == cstrExt)
            {
                CString cstrTemp = fileFinder.GetFilePath();
                vecCategoryFileList.push_back(cstrTemp.GetBuffer());
            }
        }
    }
    fileFinder.Close();
}

std::string& replace_all_distinct(std::string& str, const std::string& old_value, const std::string& new_value)
{
    for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
        if ((pos = str.find(old_value, pos)) != std::string::npos)
            str.replace(pos, old_value.length(), new_value);
        else break;
    }
    return str;
}
