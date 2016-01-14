#include<iostream>
#include<fstream>
#include<algorithm>
#include<list>
#include<iomanip>
#include<time.h>
using namespace std;

typedef unsigned short US;
typedef unsigned int UL;
typedef double RANK_TYPE;

struct PAGE
{
	US degree;
	RANK_TYPE rank;
};


const int SIZE_US = sizeof(US);
const int SIZE_UL = sizeof(UL);
const int SIZE_RANK = sizeof(RANK_TYPE);
const char* LINKS_FILE = "links.bin";
const char* DEGREE_PR = "degree.bin";
const char* PR_FILE = "PR.bin";
fstream DegreeFile, PRFile, LinksFile;

UL link_count = 0;

//计时器，性能分析使用
UL start_time, end_time;
double used_time, all_time;

/*开始计时*/
#define TIME_START all_time=0;start_time=clock();

/*阶段计时*/
#define TIME_COUNT(name) end_time = clock();\
used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;\
cout <<name << "耗时：" << used_time << "s ,累计耗时: " << all_time << "s\n";\
start_time = end_time;


inline void SetDegree(const int &n, const US & degree)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	DegreeFile.seekp(position, ios::beg).write((char*)&degree, SIZE_US);
}
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

inline PAGE GetN(const int &n)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	PAGE p;
	DegreeFile.seekg(position, ios::beg).read((char*)&p.degree, SIZE_US).read((char*)&p.rank, SIZE_RANK);
	return p;
}
inline void AddDegree(int n)
{
	US degree = GetDegree(n) + 1;
	SetDegree(n, degree);
}

UL init(const char* inputFile)
{
	ifstream input(inputFile);
	UL max_id = 0, from, to;
	while (input >> from >> to)
	{
		AddDegree(from);
		LinksFile.write((char*)&from, SIZE_UL).write((char*)&to, SIZE_UL);
		max_id = max((unsigned int)max_id, max(from, to));//记录最大ID
		++link_count;
	}
	input.close();
	return max_id;
}

void InitPR(UL max_id)
{
	RANK_TYPE base = 1.0 / max_id;
	for (UL i = 0; i < max_id; i++)
	{
		PRFile.write((char*)&base, SIZE_RANK);
	}
	base = 0;
	for (UL i = 0; i < max_id; i++)
	{
		DegreeFile.seekp(SIZE_US + i*(SIZE_US + SIZE_RANK), ios::beg).write((char*)&base, SIZE_RANK);
	}
}

void Iterate(UL max_id, RANK_TYPE beta = 0.85)
{
	RANK_TYPE rank, err, min_err = 0.000001, base = (1 - beta)*1.0 / max_id;
	UL from, to;
	PAGE page;
	int count = 0;
	do
	{
		err = 0;
		++count;
		PRFile.seekg(0);

		for (UL i = 0; i < max_id; ++i)
		{
			PRFile.read((char*)&rank, SIZE_RANK);
			err += abs(GetN(i + 1).rank - rank);
			DegreeFile.seekp(i*(SIZE_RANK + SIZE_US) + SIZE_US).write((char*)&rank, SIZE_RANK);
		}
		PRFile.seekp(0);
		for (UL i = 0; i < max_id; i++)
		{
			PRFile.write((char*)&base, SIZE_RANK);
		}
		LinksFile.seekg(0);
		for (UL i = 0; i < link_count; i++)
		{
			LinksFile.read((char*)&from, SIZE_UL).read((char*)&to, SIZE_UL);
			PRFile.seekg((to - 1)*SIZE_RANK).read((char*)&rank, SIZE_RANK);
			page = GetN(from);
			rank += page.rank / page.degree*beta;
			PRFile.seekp((to - 1)*SIZE_RANK).write((char*)&rank, SIZE_RANK);
		}
	} while (err > min_err);
	cout << "迭代次数:" << count << ",误差" << setprecision(10) << err << endl;
}

void Sort(UL max_id)
{
	struct PAGERANK { double rank = 0; UL page; };
	list<PAGERANK> sortedPR(100);
	RANK_TYPE rank;
	PRFile.seekg(0);
	UL i = 0;
	while (PRFile.read((char*)&rank, SIZE_RANK))
	{
		++i;
		for (auto it = sortedPR.begin(); it != sortedPR.end(); ++it)
		{
			if (it->rank <= rank)
			{
				PAGERANK tempPR;
				tempPR.page = i;
				tempPR.rank = rank;
				sortedPR.insert(it, tempPR);
				sortedPR.pop_back();
				break;
			}
		}
	}

	ofstream out("PR_TOP100.file.txt");
	for (auto pr : sortedPR)
	{
		cout << pr.page << "\t" << setprecision(10) << pr.rank << endl;
		out << pr.page << "\t" << setprecision(10) << pr.rank << endl;
	}
	out.close();
}

void main()
{
	ofstream out;
	out.open(LINKS_FILE); out.close();
	out.open(DEGREE_PR); out.close();
	out.open(PR_FILE); out.close();
	DegreeFile.open(DEGREE_PR, ios::binary | ios::out | ios::in);
	PRFile.open(PR_FILE, ios::binary | ios::out | ios::in);
	LinksFile.open(LINKS_FILE, ios::binary | ios::out | ios::in);
	char* data_file = "WikiData.txt";
	TIME_START;
	UL  max_id = init(data_file);
	InitPR(max_id);
	TIME_COUNT("init");
	Iterate(max_id);
	TIME_COUNT("iterate");
	Sort(max_id);
	TIME_COUNT("sort");
	DegreeFile.close();
	PRFile.close();
	LinksFile.close();
	system("pause");
}