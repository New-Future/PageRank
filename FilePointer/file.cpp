#include<iostream>
#include<fstream>
#include<algorithm>
#include<list>
#include<iomanip>
#include<time.h>
using namespace std;

typedef unsigned int UL;//ID��������
typedef unsigned short US;//������������
typedef float RANK_TYPE;//rankֵ��������
struct Degree//������Ϣ
{
	US degree;
	RANK_TYPE rank;
};

/*���峤��*/
#define SIZE_US  sizeof(US)
#define  SIZE_UL sizeof(UL)
#define SIZE_RANK sizeof(RANK_TYPE)
/*���ݽ����ļ�*/
fstream DegreeFile, PRFile, LinksFile;
UL max_id;//���ID

//��ʱ�������ܷ���ʹ��
unsigned long start_time, end_time;
double used_time, all_time;
/*��ʼ��ʱ*/
#define TIME_START all_time=0;start_time=clock()
/*�׶μ�ʱ*/
#define TIME_COUNT(name) end_time = clock();used_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;\
all_time += used_time;start_time = end_time;\
cout <<name << "��ʱ��" << used_time << "s ,�ۼƺ�ʱ: " << all_time << "s\n";\

/*���³���*/
inline void SetDegree(const int &n, const US & degree)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	DegreeFile.seekp(position, ios::beg).write((char*)&degree, SIZE_US);
}
/*��ȡ��ǰ�������޷���0*/
inline US GetDegree(const int &n)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	DegreeFile.seekg(0, ios::end);
	UL current_size = DegreeFile.tellg(); // ��ǰ�ļ���С
	/*�������С�ڵ�ǰֵ��0���򷵻ض�Ӧֵ*/
	if (position >= current_size)
	{
		return 0;
	}
	else
	{
		/*���ֵ�Ѿ�������ֱ�Ӷ�ȡ*/
		US degree;
		DegreeFile.seekg(position, ios::beg).read((char*)&degree, SIZE_US);
		return degree;
	}
}
/*��ȡIDΪN�Ķ�����rank*/
inline Degree GetN(const int &n)
{
	UL position = (n - 1)*(SIZE_US + SIZE_RANK);
	Degree p;
	DegreeFile.seekg(position, ios::beg).read((char*)&p.degree, SIZE_US).read((char*)&p.rank, SIZE_RANK);
	return p;
}

/*
��ʼ���ļ�
����������Ϣ��
д��degree��links�Ķ������ļ�
*/
UL Init(const char* inputFile)
{
	/*�����ļ�*/
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

	/*��ȡ�������ӹ�ϵ*/
	file.open(inputFile, ios::in);
	UL link_count = 0, from, to;
	while (file >> from >> to)
	{
		SetDegree(from, GetDegree(from) + 1);
		LinksFile.write((char*)&from, SIZE_UL).write((char*)&to, SIZE_UL);
		max_id = max(max_id, max(from, to));//��¼���ID
		++link_count;
	}
	file.close();

	/*��ʼ��PRֵ*/
	RANK_TYPE base = 1.0 / max_id;
	for (UL i = 0; i < max_id; i++)
	{
		DegreeFile.seekp(SIZE_US + i*(SIZE_US + SIZE_RANK), ios::beg).write((char*)&base, SIZE_RANK);
	}
	return link_count;
}
/*
ѭ������
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
		/*����PRֵΪ(1-beta)/N*/
		PRFile.seekp(0);
		for (UL i = 0; i < max_id; ++i)	PRFile.write((char*)&base, SIZE_RANK);
		/*�������ӵ���*/
		LinksFile.seekg(0);
		for (UL i = 0; i < link_count; i++)
		{
			LinksFile.read((char*)&from, SIZE_UL).read((char*)&to, SIZE_UL);
			PRFile.seekg((to - 1)*SIZE_RANK).read((char*)&rank, SIZE_RANK);
			page = GetN(from);
			rank += page.rank / page.degree*beta;
			PRFile.seekp((to - 1)*SIZE_RANK).write((char*)&rank, SIZE_RANK);
		}
		/*�������,��������*/
		err = 0;
		PRFile.seekg(0);
		for (UL i = 0; i < max_id; ++i)
		{
			PRFile.read((char*)&rank, SIZE_RANK);
			err += abs(GetN(i + 1).rank - rank);
			DegreeFile.seekp(i*(SIZE_RANK + SIZE_US) + SIZE_US).write((char*)&rank, SIZE_RANK);
		}
	} while (err > min_err);
	cout << "\n��������:" << count << ",���" << setprecision(10) << err << endl;
}
/*ǰn����*/
void Sort(const int n = 100)
{
	struct PAGERANK { double rank = 0; UL page; };
	list<PAGERANK> sortedPR(n);//����Ϊn������
	RANK_TYPE rank;
	PRFile.seekg(0);
	UL i = 0;
	/*ɨ��һ�Σ�ɸѡǰN*/
	while (PRFile.read((char*)&rank, SIZE_RANK))
	{
		++i;
		for (auto it = sortedPR.begin(); it != sortedPR.end(); ++it)
		{
			if (it->rank <= rank)
			{//����ϴ�ֵ��ɾ����β
				PAGERANK tempPR;
				tempPR.page = i;
				tempPR.rank = rank;
				sortedPR.insert(it, tempPR);
				sortedPR.pop_back();
				break;
			}
		}
	}
	/*������*/
	cout << "\nTOP" << n << "\nid\t\RANK" << endl;
	ofstream out("PR_TOPfile.txt");
	for (auto pr : sortedPR)
	{
		cout << pr.page << "\t" << setprecision(10) << pr.rank << endl;
		out << pr.page << "\t" << setprecision(12) << pr.rank << endl;
	}
	cout << "����ֵPR_TOPfile.txt\n";
	out.close();
	sortedPR.clear();
}

/* ��һ�������ļ���Ĭ��WikiData.txt;�ڶ�������betaĬ��0.85*/
void main(int argc, char *argv[])
{
	char* data_file = argc < 2 ? "WikiData.txt" : argv[1];//��һ�������ļ�
	double beta = argc >= 3 ? atof(argv[2]) : 0.85;//�ڶ�������betaֵ
	TIME_START;
	UL  link_count = Init(data_file);
	TIME_COUNT("init(��ʼ���ļ�)");
	Iterate(link_count, beta);
	TIME_COUNT("iterate(����)");
	Sort();
	TIME_COUNT("sort(�������)");
	DegreeFile.close();
	PRFile.close();
	LinksFile.close();
	if (argc <= 1)	system("pause");
}