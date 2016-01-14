/**************************************************************************
File: PageRankM.cpp
Author:SunCj & NewFuture
Date: 2015-12-21
Description: PageRank,��ȡ�ļ��е����ݣ����þ������PageRankֵ
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
//��ʱ�������ܷ���ʹ��
/*��ʼ��ʱ*/
#define TIME_START all_time=0;start_time=clock();
/*�׶μ�ʱ*/
#define TIME_COUNT(name) end_time = clock();\
	used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; \
	all_time += used_time; \
	cout << name << "��ʱ��" << used_time << "s ,�ۼƺ�ʱ: " << all_time << "s\n"; \
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
*Summary:��ȡԭʼ���ݹ������M
*Parameters:
*	file_name:�ļ���
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
	TIME_COUNT("�����ļ�");
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
*Summary:����PageRankֵ����
*Parameters:
*	a:�ڵ�
*   n:�ڵ�����
*/
void sortA(Page a[], int n)
{
	//������a����ð������
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
*Summary:����PageRankֵ
*Parameters:
*	file_name:�ļ���
*   beta:����ϵ��
*/
void PageRank(const char* file_name, double beta = 0.85)
{
	double  min_error = 1.0 / (8297 * 8297), err = 10000000, base = (1.0 - beta) / 8297;
	getM(file_name);
	//��ʼ��
	for (unsigned int i = 0; i < 8297; i++)
	{
		NewPageRank[i] = base;
		OldPageRank[i] = base;
	}
	/*����*/
	int count = 0;
	while (err>min_error)
	{
		err = 0;
		for (unsigned int i = 0; i < 8297; i++)
		{
			NewPageRank[i] = base;
		}
		/*����PRֵ*/
		for (int i = 0; i < 8297; i++)
		{
			for (int j = 0; j < 8297; j++)
			{
				NewPageRank[i] += beta*Matrix[i][j] * OldPageRank[j];
			}
			err += abs(NewPageRank[i] - OldPageRank[i]);
		}
		/*����PRֵ*/
		for (int i = 0; i < 8297; i++)
		{
			OldPageRank[i] = NewPageRank[i];
			page[i].PR = OldPageRank[i];
			page[i].id = i;
		}
		count++;
	}
	cout << "��������:" << count << ",���" << setprecision(10) << err << endl;
	TIME_COUNT("��������")/*��ʱ*/

		sortA(page, 8297);/*����*/

	ofstream out;
	out.open("PR.txt");
	for (unsigned int i = 0; i < 8297; i++)
	{
		out << page[i].id << "\t" << setprecision(10) << page[i].PR << endl;
	}
	out.close();
	cout << "ȫ��" << 8297 << "������������ PR.txt��\n";
	TIME_COUNT("�������")/*��ʱ*/
		for (int i = 0; i < 100; i++)
		{
			cout << page[i].id << "\t" << setprecision(10) << page[i].PR << endl;
		}
}

int main(int argc, char *argv[])
{
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//��һ�������ļ�
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//�ڶ�������betaֵ

	PageRank(file_name, beta);

	system("pause");
	return 0;
}