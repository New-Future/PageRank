/**************************************************************************
File: pagerank.cpp
Author: NewFuture & SunCj
Date: 2015-12-21
Description: PageRank,��ȡ�ļ��е�����
**************************************************************************/
#include<iostream>
#include<fstream>
#include<algorithm>
#include<vector>
#include<iomanip>
using namespace std;

/*ÿ��ҳ����������*/
struct Page
{
	vector<rsize_t> Links;//����,���е�����ID
	size_t src;//����ID��������
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
Page* ReadFile(const char* file_name, size_t& max_id)
{
	/*��(����)����������from->to*/
	struct Border { size_t from, to; };
	ifstream input;
	size_t from, to, count = 0;
	input.open(file_name);//���ļ�

	/*��ȡ���б�*/
	vector<Border> links;
	while (!input.eof())
	{
		input >> from >> to;
		Border b{ from,to };
		links.push_back(b);
		max_id = max((unsigned int)max_id, max(from, to));//��¼���ID
	}
	input.close();

	/*����ҳ����*/
	Page* pages = new Page[max_id];
	for each (Border link in links)
	{
		pages[link.from - 1].Links.push_back(link.to - 1);
	}
	links.clear();
	/*��ʼ��*/
	double base = 1.0 / max_id;
	for (size_t i = 0; i < max_id; i++)
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
	size_t max_id = 0;

	Page* pages = ReadFile(file_name, max_id);
	double* newPR = new double[max_id];
	double  min_error = 1.0 / (max_id*max_id), err = 10000000, base = (1.0 - beta) / max_id;
	//��ʼ��
	for (size_t i = 0; i < max_id; i++)
	{
		newPR[i] = base;
	}
	/*����*/
	int count = 0;
	while (err > min_error)
	{
		err = 0;
		/*����PRֵ*/
		for (size_t i = 0; i < max_id; i++)
		{
			for each (size_t to in pages[i].Links)
			{
				newPR[to] += beta*pages[i].Rank / pages[i].Links.size();
			}
		}
		/*����*/
		for (size_t i = 0; i < max_id; i++)
		{
			err += abs(pages[i].Rank - newPR[i]);
			pages[i].Rank = newPR[i];
			newPR[i] = base;
		}
		count++;
	}
	cout << "��������:" << count << ",���" << setprecision(10) << err << endl;

	/*�������Top10*/
	sort(pages, pages + max_id, compare);
	for (size_t i = 0; i < 10; i++)
	{
		cout << i + 1 << ends << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	/*���д���ļ�*/
	ofstream out;
	out.open("PR.txt");
	for (size_t i = 0; i < max_id; i++)
	{
		out << pages[i].src << "\t" << setprecision(10) << pages[i].Rank << endl;
	}
	out.close();
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