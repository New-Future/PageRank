/**************************************************************************
File: pagerank.cpp
Author:SunCj & NewFuture
Date: 2015-12-21
Description: PageRank,基于文件块更新的方式
指定数据以长度为N进行分块，存取

**************************************************************************/
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<list>
#include<iomanip>
#include<time.h>
#include <map>
#include <stdio.h>
using namespace std;

unsigned long start_time, end_time;
double used_time, all_time;
//计时器，性能分析使用
/*开始计时*/
#define TIME_START all_time=0;start_time=clock();
/*阶段计时*/
#define TIME_COUNT(name) end_time = clock();\
used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;\
cout <<name << "耗时：" << used_time << "s ,累计耗时: " << all_time << "s\n";\
start_time = end_time;

typedef unsigned short ID_TYPE;
typedef vector<ID_TYPE> LINKS;
typedef map<ID_TYPE, LINKS> PAGE;

/*
* 写入块
*/
inline void WriteBlock(const ID_TYPE id, LINKS& l, int N)
{
	map<ID_TYPE, LINKS>inner;
	for (auto i : l)
	{
		inner[i / N].push_back(i);
	}
	ofstream out;
	char file_name[20];
	for (auto i : inner)
	{
		sprintf(file_name, "block\\block-%d.txt", (int)i.first);
		out.open(file_name, ios::app);
		out << id << "\t" << l.size() << "\t" << i.second.size();
		for (auto pageid : i.second)
		{
			out << "\t" << pageid;
		}
		out.put('\n');
		out.close();
	}
}
/*
*Summary:读取原始数据构造Page向量
*Parameters:
*	file_name:文件名
*	&max_id : 最大ID
*Return :
*	max_id
*/
ID_TYPE InitBlock(const char* file_name, int n)
{
	ifstream input(file_name);//输入文件
	ID_TYPE from, to, max_id = 0;
	unsigned count = 0;
	/*读取所有边*/
	PAGE Pages;
	while (!input.eof() && (input >> from >> to))
	{
		Pages[from].push_back(to);
		max_id = max((ID_TYPE)max_id, max(from, to));//记录最大ID
		++count;
	}
	input.close();
	cout << file_name << "输入统计 边数【链接数量】:" << count << "有效" << Pages.size() << " ,最大ID【总页数】：" << max_id << endl;
	for (auto p : Pages)
	{
		WriteBlock(p.first, p.second, n);
	}
	Pages.clear();
	return max_id;
}

/*
*Summary:计算PAGERANK
*Parameters:
*     file_name:文件名
*     beta : 传递值beta
*Return :void
*/
void PageRank(const char* file_name, double beta = 0.85, int n = 100)
{
	//开始计时
	TIME_START;
	ID_TYPE max_id = InitBlock(file_name, n);//8297;// 
	TIME_COUNT("构建Block");
	double* PR_new = new double[max_id];
	double* PR_old = new double[max_id];
	double  min_error = 1.0 / (max_id*max_id), err, base = (1.0 - beta) / max_id;
	//初始化
	for (unsigned int i = 0; i < max_id; i++)
	{
		PR_new[i] = base;
		PR_old[i] = 0;
	}
	/*迭代*/
	int degree, size, count = 0;
	ifstream block;
	char file[20];
	ID_TYPE in_id, out_id, max = max_id / n + 1;
	do
	{
		err = 0;
		++count;
		for (ID_TYPE i = 0; i < max_id; i++)//误差统计
		{
			err += fabs(PR_new[i] - PR_old[i]);
			PR_old[i] = PR_new[i];
			PR_new[i] = (1 - beta)* base;
		}
		for (ID_TYPE i = 0; i < max; i++)//块更新
		{
			sprintf(file, "block\\block-%d.txt", (int)i);
			block.open(file);
			while (block >> in_id >> degree >> size)
			{
				while (size-- > 0)
				{
					block >> out_id;
					PR_new[out_id] += beta*PR_old[in_id] / degree;
				}
			}
			block.close();
		}
	} while (err > min_error);
	TIME_COUNT("迭代计算");/*记时*/
	cout << "迭代次数:" << count << ",误差" << setprecision(10) << err << endl;
	/*排序输出Top100*/
	struct PAGERANK
	{
		double rank = 0;
		ID_TYPE page;
	};
	list<PAGERANK> sortedPR(100);
	for (int i = 0; i < max_id; i++)
	{
		for (auto it = sortedPR.begin(); it != sortedPR.end(); ++it)
		{
			if (it->rank <= PR_new[i])
			{
				PAGERANK tempPR;
				tempPR.page = i;
				tempPR.rank = PR_new[i];
				sortedPR.insert(it, tempPR);
				sortedPR.pop_back();
				break;
			}
		}
	}
	ofstream out("PR_TOP100.block.txt");
	for (auto pr : sortedPR)
	{
		cout << pr.page << "\t" << setprecision(10) << pr.rank << endl;
		out << pr.page << "\t" << setprecision(12) << pr.rank << endl;
	}
	out.close();
	cout << "结果已输出到 PR_TOP100.block.txt中\n";
	TIME_COUNT("排序输出")/*记时*/
}

/*
* 第一个参数文件名默认WikiData.txt
* 第二个参数beta默认0.85
*/
int main(int argc, char *argv[])
{
	system("mkdir block");
	system("erase block /Q");
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//第一个参数文件
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//第二个参数beta值
	int N = (argc >= 4) ? atoi(argv[3]) : 150;
	PageRank(file_name, beta, N);
	if (argc <= 1)	system("pause");
	return 0;
}