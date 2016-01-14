/**************************************************************************
File: pagerank.cpp
Author:SunCj & NewFuture
Date: 2015-12-21
Description: PageRank,
读取文件中的数据
**************************************************************************/
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<iomanip>
#include<time.h>
using namespace std;

typedef float RANK_TYPE;
typedef unsigned short ID_TYPE;
/*每个页的数据类型*/
struct Page
{
	vector<unsigned int> Links;//外链,所有的外链ID
	unsigned int src;//自身ID用于排序
	RANK_TYPE Rank = 0;//PR值
};

unsigned long start_time, end_time;
RANK_TYPE used_time, all_time;
//计时器，性能分析使用
/*开始计时*/
#define TIME_START all_time=0;start_time=clock();
/*阶段计时*/
#define TIME_COUNT(name) end_time = clock();\
used_time = (RANK_TYPE)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;\
cout <<name << "耗时：" << used_time << "s ,累计耗时: " << all_time << "s\n";\
start_time = end_time;



/*Page大小比较方法用于排序*/
bool compare(Page &p1, Page &p2) { return p1.Rank > p2.Rank; }

/*
*Summary:读取原始数据构造Page向量
*Parameters:
*	file_name:文件名
*	&max_id : 最大ID
*Return :
*	Pages* 所有页，长度为max_id
*/
Page* Init(const char* file_name, unsigned int& max_id)
{
	/*边(链接)的数据类型from->to*/
	struct Border { unsigned int from, to; };
	ifstream input;
	unsigned int from, to, count = 0;
	input.open(file_name);//打开文件

	/*读取所有边*/
	vector<Border> links;
	while (!input.eof() && (input >> from >> to))
	{
		Border b{ from,to };
		links.push_back(b);
		max_id = max((unsigned int)max_id, max(from, to));//记录最大ID
		++count;
	}
	input.close();
	cout << file_name << "输入统计 边数【链接数量】:" << count << " ,最大ID【总页数】：" << max_id << endl;

	/*创建页向量*/
	Page* pages = new Page[max_id];
	for each (Border link in links)
	{
		pages[link.from - 1].Links.push_back(link.to - 1);
	}
	links.clear();
	/*初始化*/
	RANK_TYPE base = 1.0 / max_id;
	for (unsigned int i = 0; i < max_id; i++)
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
void PageRank(const char* file_name, RANK_TYPE beta = 0.85)
{
	unsigned int max_id = 0;
	//开始计时
	TIME_START;
	Page* pages = Init(file_name, max_id);
	TIME_COUNT("读入文件");
	RANK_TYPE* newPR = new RANK_TYPE[max_id];
	RANK_TYPE  min_error = 1.0 / (max_id*max_id), err = 10000000, base = (1.0 - beta) / max_id;
	//初始化
	for (unsigned int i = 0; i < max_id; i++)
	{
		newPR[i] = base;
	}
	/*迭代*/
	int count = 0;
	while (err > min_error)
	{
		err = 0;
		/*计算PR值*/
		for (unsigned int i = 0; i < max_id; i++)
		{
			for (unsigned int to : pages[i].Links)
			{
				newPR[to] += beta*pages[i].Rank / pages[i].Links.size();
			}
		}
		/*更新*/
		for (unsigned int i = 0; i < max_id; i++)
		{
			err += abs(pages[i].Rank - newPR[i]);
			pages[i].Rank = newPR[i];
			newPR[i] = base;
		}
		count++;
	}
	cout << "迭代次数:" << count << ",误差" << setprecision(10) << err << endl;

	TIME_COUNT("迭代计算");/*记时*/

	/*排序输出Top100*/
	sort(pages, pages + max_id, compare);
	cout << "\nTOP100\t ID \t RANK\n";
	for (unsigned int i = 0; i < 100; i++)
	{
		cout << i + 1 << "\t" << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	/*结果写入文件*/
	ofstream out;
	out.open("PR.txt");
	for (unsigned int i = 0; i < max_id; i++)
	{
		out << pages[i].src << "\t" << setprecision(12) << pages[i].Rank << endl;
	}
	out.close();
	cout << "全部" << max_id << "条结果已输出到 PR.txt中\n";
	TIME_COUNT("排序输出")/*记时*/
}

/*
* 第一个参数文件名默认WikiData.txt
* 第二个参数beta默认0.85
*/
int main(int argc, char *argv[])
{
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//第一个参数文件
	RANK_TYPE beta = argc >= 3 ? atof(argv[2]) : 0.85;//第二个参数beta值
	PageRank(file_name, beta);
	if (argc <= 1)	system("pause");
	return 0;
}