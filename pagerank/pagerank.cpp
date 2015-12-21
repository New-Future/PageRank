/**************************************************************************
File: pagerank.cpp
Author: NewFuture & SunCj
Date: 2015-12-21
Description: PageRank,读取文件中的数据
**************************************************************************/
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<iomanip>
using namespace std;

/*每个页的数据类型*/
struct Page
{
	vector<rsize_t> Links;//外链,所有的外链ID
	size_t src;//自身ID用于排序
	double Rank = 0;//PR值
};

/*Page大小比较方法*/
bool compare(Page &p1, Page &p2) { return p1.Rank > p2.Rank; }

/*
*Summary:读取原始数据构造Page向量
*Parameters:
*	file_name:文件名
*	&max_id : 最大ID
*Return :
*	Pages* 所有页，长度为max_id
*/
Page* ReadFile(const char* file_name, size_t& max_id)
{
	/*边(链接)的数据类型from->to*/
	struct Border { size_t from, to; };
	ifstream input;
	size_t from, to, count = 0;
	input.open(file_name);//打开文件

	/*读取所有边*/
	vector<Border> links;
	while (!input.eof())
	{
		input >> from >> to;
		Border b{ from,to };
		links.push_back(b);
		max_id = max((unsigned int)max_id, max(from, to));//记录最大ID
	}
	input.close();

	/*创建页向量*/
	Page* pages = new Page[max_id];
	for each (Border link in links)
	{
		pages[link.from - 1].Links.push_back(link.to - 1);
	}
	links.clear();
	/*初始化*/
	double base = 1.0 / max_id;
	for (size_t i = 0; i < max_id; i++)
	{
		pages[i].src = i + 1;
		pages[i].Rank = base;
	}
	return pages;
}

/*
*Summary:计算PAGERANK
*Parameters:
*     file_name:文件名
*     beta : 传递值beta
*Return :void
*/
void PageRank(const char* file_name, double beta = 0.85)
{
	size_t max_id = 0;

	Page* pages = ReadFile(file_name, max_id);
	double* newPR = new double[max_id];
	double  min_error = 1.0 / (max_id*max_id), err = 10000000, base = (1.0 - beta) / max_id;
	//初始化
	for (size_t i = 0; i < max_id; i++)
	{
		newPR[i] = base;
	}
	/*迭代*/
	int count = 0;
	while (err > min_error)
	{
		err = 0;
		/*计算PR值*/
		for (size_t i = 0; i < max_id; i++)
		{
			for each (size_t to in pages[i].Links)
			{
				newPR[to] += beta*pages[i].Rank / pages[i].Links.size();
			}
		}
		/*更新*/
		for (size_t i = 0; i < max_id; i++)
		{
			err += abs(pages[i].Rank - newPR[i]);
			pages[i].Rank = newPR[i];
			newPR[i] = base;
		}
		count++;
	}
	cout << "迭代次数:" << count << ",误差" << setprecision(10) << err << endl;

	/*排序输出Top10*/
	sort(pages, pages + max_id, compare);
	for (size_t i = 0; i < 10; i++)
	{
		cout << i + 1 << ends << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	/*结果写入文件*/
	ofstream out;
	out.open("PR.txt");
	for (size_t i = 0; i < max_id; i++)
	{
		out << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	out.close();
}

/*
* 第一个参数文件名默认WikiData.txt
* 第二个参数beta默认0.85
*/
int main(int argc, char *argv[])
{
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//第一个参数文件
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//第二个参数beta值
	PageRank(file_name, beta);
	system("pause");
	return 0;
}