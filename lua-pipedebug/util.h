#pragma once

// ��ָ��λ�ÿ�ʼ���׸����ǿո��λ�ã��� ֱ�������ո����
std::string parse_segment(const std::string& val);

std::string parse_segment(const std::string& val,int &begin);

void filter_chars(std::string& val, const std::string &filters);