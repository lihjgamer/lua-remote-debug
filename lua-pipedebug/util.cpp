#include "stdafx.h"
#include "util.h"



// ��ָ��λ�ÿ�ʼ���׸����ǿո��λ�ã��� ֱ�������ո����
std::string parse_segment(const std::string& val,int &begin)
{
	int len = val.length();
	std::string ret;
	while(begin < len && val[begin] == ' ') ++begin;
	for (;begin < len && val[begin] != ' ';++begin)
	{
		ret += val[begin];
	}

	return ret;
}

std::string parse_segment(const std::string& val)
{
	int npos = 0;
	return parse_segment(val,npos);
}


void filter_chars(std::string& val, const std::string &filters)
{
	int begin = 0;
	for (;begin < (int)val.size();++begin)
	{
		bool bcontinue = false;
		for (int i = 0; i < (int)filters.size(); ++i)
		{
			if (val[begin] == filters[i])
			{
				bcontinue = true;
				break;
			}
		}

		if (!bcontinue)
		{
			break;
		}
	}
	if (begin >= (int)val.size())
	{
		val.clear();
	}
	else
	{
		val = val.substr(begin);
	}
}

