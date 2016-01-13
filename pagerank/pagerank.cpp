/**************************************************************************
File: pagerank.cpp
Author:SunCj & NewFuture
Date: 2015-12-21
Description: PageRank,��ȡ�ļ��е�����
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
used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;\
cout <<name << "��ʱ��" << used_time << "s ,�ۼƺ�ʱ: " << all_time << "s\n";\
start_time = end_time;

/*ÿ��ҳ����������*/
struct Page
{
	vector<unsigned int> Links;//����,���е�����ID
	unsigned int src;//����ID��������
	double Rank = 0;//PRֵ
};

/*Page��С�ȽϷ���*/
bool compare(Page &p1, Page &p2) { return p1.Rank > p2.Rank; }

/*
*Summary:��ȡԭʼ���ݹ���Page����
*Parameters:
*	file_name:�ļ���
*	&max_id : ���ID
*Return :
*	Pages* ����ҳ������Ϊmax_id
*/
Page* ReadFile(const char* file_name, unsigned int& max_id)
{
	/*��(����)����������from->to*/
	struct Border { unsigned int from, to; };
	ifstream input;
	unsigned int from, to, count = 0;
	input.open(file_name);//���ļ�

	/*��ȡ���б�*/
	vector<Border> links;
	while (!input.eof() && (input >> from >> to))
	{
		Border b{ from,to };
		links.push_back(b);
		max_id = max((unsigned int)max_id, max(from, to));//��¼���ID
		++count;
	}
	input.close();
	cout << file_name << "����ͳ�� ����������������:" << count << " ,���ID����ҳ������" << max_id << endl;

	/*����ҳ����*/
	Page* pages = new Page[max_id];
	for each (Border link in links)
	{
		pages[link.from - 1].Links.push_back(link.to - 1);
	}
	links.clear();
	/*��ʼ��*/
	double base = 1.0 / max_id;
	for (unsigned int i = 0; i < max_id; i++)
	{
		pages[i].src = i + 1;
		pages[i].Rank = base;
	}
	return pages;
}

/*
*Summary:����PAGERANK
*Parameters:
*     file_name:�ļ���
*     beta : ����ֵbeta
*Return :void
*/
void PageRank(const char* file_name, double beta = 0.85)
{
	unsigned int max_id = 0;

	//��ʼ��ʱ
	TIME_START;
	Page* pages = ReadFile(file_name, max_id);
	TIME_COUNT("�����ļ�");
	double* newPR = new double[max_id];
	double  min_error = 1.0 / (max_id*max_id), err = 10000000, base = (1.0 - beta) / max_id;
	//��ʼ��
	for (unsigned int i = 0; i < max_id; i++)
	{
		newPR[i] = base;
	}
	/*����*/
	int count = 0;
	while (err > min_error)
	{
		err = 0;
		/*����PRֵ*/
		for (unsigned int i = 0; i < max_id; i++)
		{
			for (unsigned int to : pages[i].Links)
			{
				newPR[to] += beta*pages[i].Rank / pages[i].Links.size();
			}
		}
		/*����*/
		for (unsigned int i = 0; i < max_id; i++)
		{
			err += abs(pages[i].Rank - newPR[i]);
			pages[i].Rank = newPR[i];
			newPR[i] = base;
		}
		count++;
	}
	cout << "��������:" << count << ",���" << setprecision(10) << err << endl;

	TIME_COUNT("��������")/*��ʱ*/

	/*�������Top10*/
		sort(pages, pages + max_id, compare);
	cout << "\nTOP100\t ID \t RANK\n";
	for (unsigned int i = 0; i < 100; i++)
	{
		cout << i + 1 << "\t" << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	/*���д���ļ�*/
	ofstream out;
	out.open("PR.txt");
	for (unsigned int i = 0; i < max_id; i++)
	{
		out << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	out.close();
	cout << "ȫ��" << max_id << "������������ PR.txt��\n";
	TIME_COUNT("�������")/*��ʱ*/
}

/*
* ��һ�������ļ���Ĭ��WikiData.txt
* �ڶ�������betaĬ��0.85
*/
int main(int argc, char *argv[])
{
	char* file_name = argc < 2 ? "WikiData.txt" : argv[1];//��һ�������ļ�
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//�ڶ�������betaֵ
	PageRank(file_name, beta);
	system("pause");
	return 0;
}