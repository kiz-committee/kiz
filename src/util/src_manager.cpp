/**
 * @file src_manager.cpp
 * @brief 源文件管理器（Source File Manager）核心
 * 管理Kiz代码源文件
 * 并给予错误报告器源文件错误代码的切片
 * @author azhz1107cat
 */

#include "src_manager.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../kiz.hpp"


namespace err {
/**
 * @brief 正确切分文件内容并去掉可能的\r
 * @param filecon 形如<content_1>\n<content_2>\n....<content_n>\n的一串字符串，以正确分割
 * @result 返回正确切分的内容
 */
std::vector<std::string> slice_file_content(std::string filecon) {
    std::vector<std::string> sliced_content;
    std::string nowline;
    for (const auto& charact: filecon) {
        if (charact == '\n') {
            sliced_content.emplace_back(nowline);
            nowline = "";
        }
        else
            nowline += charact;
    }
    // 清除 \r
    std::vector<std::string> result;
    for (const auto& line: sliced_content) {
        if (line.ends_with("\r")) {
            result.emplace_back(line.substr(0, line.length() - 1));
        } else {
            result.emplace_back(line);
        }
    }
    return result;
}

std::unordered_map<std::string, std::string> SrcManager::opened_files;

bool SrcManager::is_valid_file_range(
    const int &src_line_start,
    const int &src_line_end,
    const size_t total_lines
) {
    return src_line_start >= 1
        && src_line_end >= 1
        && src_line_start <= src_line_end
        && static_cast<size_t>(src_line_end) <= total_lines;
}

/**
 * @brief 从指定文件中提取指定行范围的内容（行号从1开始）
 * @param src_path 文件路径
 * @param src_line_start 起始行号（闭区间）
 * @param src_line_end 结束行号（闭区间）
 * @return 提取的行内容（每行保留原始换行符，无效范围返回空字符串）
 */
std::string SrcManager::get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end) {
    DEBUG_OUTPUT("get slice");
    // 先获取完整文件内容（依赖缓存机制）
    std::string file_content = get_file_by_path(src_path);
    DEBUG_OUTPUT("file_content is:" + file_content);

    DEBUG_OUTPUT("try to slice [" << src_line_start << " - " << src_line_end << "]");
    // 按行分割文件内容（兼容 Windows \r\n 和 Linux \n 换行符）
    std::stringstream content_stream(file_content);
    std::string line;

    // 使get_slice能读到没有换行符结尾的串
    file_content += "\n";
    DEBUG_OUTPUT("Added trailing newline to file content");

    std::string repr_file_content;
    for (const auto charact: file_content) {
        if (charact == '\n') {
            repr_file_content += "\\n";
        } else {
            repr_file_content += charact;
        }
    }
    DEBUG_OUTPUT("repr_file_content: " + repr_file_content);

    /*while (std::getline(content_stream, line)) {
        // 移除 Windows 换行符残留的 \r（std::getline 会自动去掉 \n，但不会处理 \r）
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.emplace_back(std::move(line));
    }*/
    std::vector<std::string> lines = slice_file_content(file_content);

    int sumline = 0;
    for (const std::string& k : lines) {

        if (k == "") DEBUG_OUTPUT("k is empty");
        else if (k == "\n") DEBUG_OUTPUT("k is \\n");
        else DEBUG_OUTPUT("k is normal: " << k);
        sumline ++;
    }
    DEBUG_OUTPUT("Finally, got " << sumline << " lines");


    // 校验行号范围有效性（行号从1开始，且起始行 ≤ 结束行）
    const size_t total_lines = lines.size();
    DEBUG_OUTPUT("There is/are " << static_cast<int>(total_lines) << " line(s)");
    if (!is_valid_file_range(src_line_start, src_line_end, total_lines)) {
        DEBUG_OUTPUT( "[Warning] Invalid line range: start=" << src_line_start
                  << ", end=" << src_line_end << " (total lines: " << total_lines << ")\n");
        return "";
    } else DEBUG_OUTPUT("Valid line range");

    // 提取目标行范围（转换为0-based索引）
    std::stringstream slice_stream;
    const size_t start_idx = static_cast<size_t>(src_line_start) - 1;
    const size_t end_idx = static_cast<size_t>(src_line_end) - 1;
    for (size_t i = start_idx; i <= end_idx; ++i) {
        slice_stream << lines[i];
        // 除最后一行外，保留原始换行符（还原文件格式）
        if (i != end_idx) {
            slice_stream << '\n';
        }
    }

    return slice_stream.str();
}

/**
 * @brief 线程安全获取文件内容（优先从缓存读取，未命中则新打开）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出异常）
 */
std::string SrcManager::get_file_by_path(std::string path) {
    DEBUG_OUTPUT("get_file_by_path");
    // 检查缓存是否已存在该文件
    DEBUG_OUTPUT(path);
    DEBUG_OUTPUT("finding");
    const auto it = opened_files.find(path);
    if (it != opened_files.end()) {
        DEBUG_OUTPUT("in opened files !");

        // 先打印 key 确保是对的
        DEBUG_OUTPUT("found key: " + it->first);

        // 打印 value 的长度
        DEBUG_OUTPUT("value length: " + std::to_string(it->second.size()));

        // 打印 value 的前几个字符，避免格式化问题
        std::string safe_content = it->second;
        if (safe_content.size() > 20) {
            safe_content = safe_content.substr(0, 20) + "...";
        }
        DEBUG_OUTPUT(std::string("content is: ") + safe_content);

        return it->second;
    }
    DEBUG_OUTPUT("no found");

    // 缓存未命中，新打开文件并加入缓存
    std::string file_content = read_file(path);
    DEBUG_OUTPUT(file_content);
    DEBUG_OUTPUT(path+" "+file_content);
    opened_files.emplace(path, file_content);
    DEBUG_OUTPUT("finish get_file_by_path");
    return file_content;
}

/**
 * @brief 打开Kiz文件并读取内容（不直接对外暴露，由get_file_by_path调用）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出std::runtime_error）
 */
std::string SrcManager::read_file(const std::string& path) {
    DEBUG_OUTPUT("read_file: " + path);
    std::ifstream kiz_file(path, std::ios::binary);
    if (!kiz_file.is_open()) {
        throw KizStopRunningSignal("Failed to open file: " + path);
    }

    kiz_file.seekg(0, std::ios::end);
    const std::streampos file_size = kiz_file.tellg();
    if (file_size < 0) {
        throw KizStopRunningSignal("Failed to get file size: " + path);
    }

    std::string file_content;
    file_content.resize(file_size);

    kiz_file.seekg(0, std::ios::beg);
    kiz_file.read(&file_content[0], file_size);

    if (!kiz_file) {
        throw KizStopRunningSignal("Failed to read file: " + path);
    }

    DEBUG_OUTPUT("File read successfully, size: " + std::to_string(file_size));
    DEBUG_OUTPUT(file_content);
    DEBUG_OUTPUT("finish read_file");
    return file_content;
}

} // namespace err