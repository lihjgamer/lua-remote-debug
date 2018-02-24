#pragma once

// 从指定位置开始（首个不是空格的位置）起 直到遇到空格结束
std::string parse_segment(const std::string& val);

std::string parse_segment(const std::string& val,int &begin);

void filter_chars(std::string& val, const std::string &filters);