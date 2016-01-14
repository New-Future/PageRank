#include<iostream>
#include<fstream>
#include<algorithm>
#include<list>
#include<iomanip>
#include<time.h>
using namespace std;

typedef unsigned int UL;//ID数据类型
typedef unsigned short US;//度数数据类型
typedef float RANK_TYPE;//rank值数据类型
struct Degree//出度信息
{
	US degree;
	RANK_TYPE rank;
};

/*定义长度*/
#define SIZE_US  sizeof(US)
#define  SIZE_UL sizeof(UL)
#define SIZE_RANK sizeof(RANK_TYPE)
/*数据交换文件*/
fstream DegreeFile, PRFile, LinksFile;
UL max_id;//最大ID

//计时器，性能分析使用
unsigned long start_time, end_time;
double used_time, all_time;
/*开始计时*/
#define TIME_START all_time=0;start_time=clock()
/*阶段计时*/
#define TIME_COUNT(name) end_time = clock();used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;start_time = end_time;\
cout <<name << "耗时：" << used_time << "s ,累计耗时: " << all_time << "s\n";\

/*更新出度*/
inline void SetDegree(const int &n, const US & degree)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	DegreeFile.seekp(position, ios::beg).write((char*)&degree, SIZE_US);
}
/*获取当前度数，无返回0*/
inline US GetDegree(const int &n)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	DegreeFile.seekg(0, ios::end);
	UL current_size = DegreeFile.tellg(); // 当前文件大小
	/*如果长度小于当前值填0否则返回对应值*/
	if (position >= current_size)
	{
		return 0;
	}
	else
	{
		/*这个值已经存在则直接读取*/
		US degree;
		DegreeFile.seekg(position, ios::beg).read((char*)&degree, SIZE_US);
		return degree;
	}
}
/*获取ID为N的度数和rank*/
inline Degree GetN(const int &n)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	Degree p;
	DegreeFile.seekg(position, ios::beg).read((char*)&p.degree, SIZE_US).read((char*)&p.rank, SIZE_RANK);
	return p;
}

/*
初始化文件
根据输入信息，
写入degree和links的二进制文件
*/
UL Init(const char* inputFile)
{
	/*创建文件*/
	const char* LINKS_FILE = "_links.bin";
	const char* DEGREE_PR = "_degree.bin";
	const char* PR_FILE = "_pr.bin";
	fstream file;
	file.open(LINKS_FILE, ios::out); file.close();
	file.open(DEGREE_PR, ios::out); file.close();
	file.open(PR_FILE, ios::out); file.close();
	DegreeFile.open(DEGREE_PR, ios::binary | ios::out | ios::in);
	PRFile.open(PR_FILE, ios::binary | ios::out | ios::in);
	LinksFile.open(LINKS_FILE, ios::binary | ios::out | ios::in);

	/*读取输入链接关系*/
	file.open(inputFile, ios::in);
	UL link_count = 0, from, to;
	while (file >> from >> to)
	{
		SetDegree(from, GetDegree(from) + 1);
		LinksFile.write((char*)&from, SIZE_UL).write((char*)&to, SIZE_UL);
		max_id = max(max_id, max(from, to));//记录最大ID
		++link_count;
	}
	file.close();

	/*初始化PR值*/
	RANK_TYPE base = 1.0 / max_id;
	for (UL i = 0; i < max_id; i++)
	{
		DegreeFile.seekp(SIZE_US + i*(SIZE_US + SIZE_RANK), ios::beg).write((char*)&base, SIZE_RANK);
	}
	return link_count;
}
/*
循环迭代
*/
void Iterate(UL link_count, RANK_TYPE beta)
{
	RANK_TYPE rank, err, min_err = 1.0 / link_count, base = (1 - beta)*1.0 / max_id;
	UL from, to;
	Degree page;
	int count = 0;
	do
	{
		++count;
		/*重置PR值为(1-beta)/N*/
		PRFile.seekp(0);
		for (UL i = 0; i < max_id; ++i)	PRFile.write((char*)&base, SIZE_RANK);
		/*逐条链接迭代*/
		LinksFile.seekg(0);
		for (UL i = 0; i < link_count; i++)
		{
			LinksFile.read((char*)&from, SIZE_UL).read((char*)&to, SIZE_UL);
			PRFile.seekg((to - 1)*SIZE_RANK).read((char*)&rank, SIZE_RANK);
			page = GetN(from);
			rank += page.rank / page.degree*beta;
			PRFile.seekp((to - 1)*SIZE_RANK).write((char*)&rank, SIZE_RANK);
		}
		/*计算误差,更新数据*/
		err = 0;
		PRFile.seekg(0);
		for (UL i = 0; i < max_id; ++i)
		{
			PRFile.read((char*)&rank, SIZE_RANK);
			err += abs(GetN(i + 1).rank - rank);
			DegreeFile.seekp(i*(SIZE_RANK + SIZE_US) + SIZE_US).write((char*)&rank, SIZE_RANK);
		}
	} while (err > min_err);
	cout << "\n迭代次数:" << count << ",误差" << setprecision(10) << err << endl;
}
/*前n排序*/
void Sort(const int n = 100)
{
	struct PAGERANK { double rank = 0; UL page; };
	list<PAGERANK> sortedPR(n);//长度为n的链表
	RANK_TYPE rank;
	PRFile.seekg(0);
	UL i = 0;
	/*扫描一次，筛选前N*/
	while (PRFile.read((char*)&rank, SIZE_RANK))
	{
		++i;
		for (auto it = sortedPR.begin(); it != sortedPR.end(); ++it)
		{
			if (it->rank <= rank)
			{//插入较大值，删除行尾
				PAGERANK tempPR;
				tempPR.page = i;
				tempPR.rank = rank;
				sortedPR.insert(it, tempPR);
				sortedPR.pop_back();
				break;
			}
		}
	}
	/*输出结果*/
	cout << "\nTOP" << n << "\nid\t\RANK" << endl;
	ofstream out("PR_TOPfile.txt");
	for (auto pr : sortedPR)
	{
		cout << pr.page << "\t" << setprecision(10) << pr.rank << endl;
		out << pr.page << "\t" << setprecision(12) << pr.rank << endl;
	}
	cout << "保存值PR_TOPfile.txt\n";
	out.close();
	sortedPR.clear();
}

/* 第一个参数文件名默认WikiData.txt;第二个参数beta默认0.85*/
void main(int argc, char *argv[])
{
	char* data_file = argc < 2 ? "WikiData.txt" : argv[1];//第一个参数文件
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//第二个参数beta值
	TIME_START;
	UL  link_count = Init(data_file);
	TIME_COUNT("init(初始化文件)");
	Iterate(link_count, beta);
	TIME_COUNT("iterate(迭代)");
	Sort();
	TIME_COUNT("sort(排序输出)");
	DegreeFile.close();
	PRFile.close();
	LinksFile.close();
	if (argc <= 1)	system("pause");
}