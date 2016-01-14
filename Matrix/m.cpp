/**************************************************************************
File: PageRankM.cpp
Author:SunCj & NewFuture
Date: 2015-12-21
Description: PageRank,读取文件中的数据，利用矩阵计算PageRank值
**************************************************************************/
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<iomanip>
#include<time.h>
using namespace std;

unsigned long start_time, end_time;
double used_time, all_time;
//计时器，性能分析使用
/*开始计时*/
#define TIME_START all_time=0;start_time=clock();
/*阶段计时*/
#define TIME_COUNT(name) end_time = clock();\
	used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; \
	all_time += used_time; \
	cout << name << "耗时：" << used_time << "s ,累计耗时: " << all_time << "s\n"; \
	start_time = end_time;
double Matrix[8297][8297];
double NewPageRank[8297];
double OldPageRank[8297];
struct Page
{
	double PR;
	int id;
};
Page page[8297];

/*
*Summary:读取原始数据构造矩阵M
*Parameters:
*	file_name:文件名
*/
void getM(const char* file_name)
{
	ifstream input;
	input.open(file_name);
	unsigned int from, to;
	for (int i = 0; i < 8297; i++)
	{
		for (int j = 0; j < 8297; j++)
		{
			Matrix[i][j] = 0;
		}
	}
	while (!input.eof() && (input >> from >> to))
	{
		Matrix[to][from] = 1;
	}
	input.close();
	TIME_COUNT("读入文件");
	int degree[8297];
	for (int i = 0; i < 8297; i++)
	{
		degree[i] = 0;
	}
	for (int i = 0; i < 8297; i++)
	{
		for (int j = 0; j < 8297; j++)
		{
			if (Matrix[j][i] == 1)
			{
				degree[i] = degree[i] + 1;
			}
		}
	}
	for (int i = 0; i < 8297; i++)
	{
		if (degree[i] != 0)
		{
			for (int j = 0; j < 8297; j++)
			{
				if (Matrix[j][i] == 1)
				{
					Matrix[j][i] = Matrix[j][i] / degree[i];
				}
			}
		}
	}

}
/*
*Summary:根据PageRank值排序
*Parameters:
*	a:节点
*   n:节点总数
*/
void sortA(Page a[], int n)
{
	//对数组a进行冒泡排序
	int i, j;
	for (i = 0; i<n - 1; i++)
	{
		for (j = 0; j<n - 1 - i; j++)
		{
			if (a[j + 1].PR>a[j].PR)
			{
				swap(a[j + 1], a[j]);
			}
		}
	}
}


/*
*Summary:计算PageRank值
*Parameters:
*	file_name:文件名
*   beta:阻尼系数
*/
void PageRank(const char* file_name, double beta = 0.85)
{
	double  min_error = 1.0 / (8297 * 8297), err = 10000000, base = (1.0 - beta) / 8297;
	getM(file_name);
	//初始化
	for (unsigned int i = 0; i < 8297; i++)
	{
		NewPageRank[i] = base;
		OldPageRank[i] = base;
	}
	/*迭代*/
	int count = 0;
	while (err>min_error)
	{
		err = 0;
		for (unsigned int i = 0; i < 8297; i++)
		{
			NewPageRank[i] = base;
		}
		/*计算PR值*/
		for (int i = 0; i < 8297; i++)
		{
			for (int j = 0; j < 8297; j++)
			{
				NewPageRank[i] += beta*Matrix[i][j] * OldPageRank[j];
			}
			err += abs(NewPageRank[i] - OldPageRank[i]);
		}
		/*更新PR值*/
		for (int i = 0; i < 8297; i++)
		{
			OldPageRank[i] = NewPageRank[i];
			page[i].PR = OldPageRank[i];
			page[i].id = i;
		}
		count++;
	}
	cout << "迭代次数:" << count << ",误差" << setprecision(10) << err << endl;
	TIME_COUNT("迭代计算")/*记时*/

		sortA(page, 8297);/*排序*/

	ofstream out;
	out.open("PR.txt");
	for (unsigned int i = 0; i < 8297; i++)
	{
		out << page[i].id << "\t" << setprecision(10) << page[i].PR << endl;
	}
	out.close();
	cout << "全部" << 8297 << "条结果已输出到 PR.txt中\n";
	TIME_COUNT("排序输出")/*记时*/
		for (int i = 0; i < 100; i++)
		{
			cout << page[i].id << "\t" << setprecision(10) << page[i].PR << endl;
		}
}

int main(int argc, char *argv[])
{
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//第一个参数文件
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//第二个参数beta值

	PageRank(file_name, beta);

	system("pause");
	return 0;
}