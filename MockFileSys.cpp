// MockFileSys.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<stdlib.h>
#include<iostream>
#include<vector>
#include<string>
#include<Ctime>
#include<time.h>
#include<cstdio>
#include <sstream>
#include <Windows.h>
using namespace std;


typedef struct
{
	char filename[20];
	int type;//0Ŀ¼��1�ļ�
	int mode; // �ļ�Ȩ��0 - readonly;  1 - writeonly;  2 - read / write
	int length;//�ļ�����(���ֽ�������)  
	int addr;//���Ϊ�ļ�������DB�±ꣻ���Ϊ�ļ��У�������-1
	int parent;//��һ��Ŀ¼����Ŀ¼-1���ļ�-1
	char day[11];//��һ��\0
	char tim[6];
}UFD;  //�ļ���Ŀ¼   57B


typedef struct
{
	int next_num;//��һ����ţ����ܲ���������û����һ��-1
	int is_data;//���û��ǲ�����
	char data[256];

}DB;  //���ݿ�  һά����   264B

//����
extern int Headnum;
extern int Middlenum;
extern int presentdir;//��ǰĿ¼
extern UFD FileInput;
extern DB DBInput;
extern vector <UFD> FileInfo;
extern vector<DB> FileDB;
extern string path;

//����
int Headnum;
int Middlenum;
int presentdir = -1;//�Ӹ�Ŀ¼��ʼ
UFD FileInput;
DB DBInput;
vector <UFD> FileInfo;  //һά��̬����
vector<DB> FileDB;
string path = "\\";



void initFiletoRom()
{
	FileInfo.clear();
	FileDB.clear();
	FILE *fd;
	if ((fd = fopen("disk.txt", "r")) == NULL)
	{
		cout << "�����ж���ʧ�ܣ�" << endl;
		return;
	}

	fscanf(fd, "%d", &Headnum);
	int alreadynum;
	int ret;
	alreadynum = 0;

	//��ʼ���ļ���Ϣ
	for (int i = 0; i < Headnum; i++)
	{
		if ((ret = fscanf(fd, "%s %d %d %d %d %d %s %s", &FileInput.filename, &FileInput.type, &FileInput.mode, &FileInput.length, &FileInput.addr, &FileInput.parent, &FileInput.day, &FileInput.tim)) != -1)
		{
			FileInfo.push_back(FileInput);
			//cout << FileInput.filename << FileInput.type << FileInput.mode << FileInput.length << FileInput.addr << FileInput.parent << FileInput.day << " " << FileInput.tim << endl;
		}

	}
	alreadynum = 0;
	char Tempbuf[256];  //һ���ļ������256
	char c;
	fscanf(fd, "%d", &Middlenum);
	while (alreadynum < Middlenum)
	{
		memset(Tempbuf, 0, sizeof(Tempbuf));//ȫ����ʼ��0
		if ((ret = fscanf(fd, "%d %d", &DBInput.next_num, &DBInput.is_data)) != -1)
		{
			if (DBInput.is_data != -1)
				fscanf(fd,"%s",&DBInput.data);
			else
			{
				fgets(Tempbuf, 256, fd);
				strcpy(DBInput.data, Tempbuf);
			}
			FileDB.push_back(DBInput);
			//cout << DBInput.data;
		}
		else
		{
			break;
		}
		alreadynum++;
	}
	fclose(fd);
}

void out_to_file()  //����д
{
	FILE* fd;
	fd = fopen("disk.txt", "w");
	fprintf(fd, "%d", FileInfo.size());//Headnum
	fprintf(fd, "\n");
	for (int i = 0; i < FileInfo.size(); i++)
	{
		fprintf(fd, "%s %d %d %d %d %d %s %s", FileInfo[i].filename, FileInfo[i].type, FileInfo[i].mode, FileInfo[i].length, FileInfo[i].addr, FileInfo[i].parent, FileInfo[i].day, FileInfo[i].tim);
		fprintf(fd, "\n");
	}
	fprintf(fd, "%d", FileDB.size());//Middlenum
	fprintf(fd, "\n");
	for (int i = 0; i < FileDB.size(); i++)
	{
		fprintf(fd, "%d %d%c", FileDB[i].next_num, FileDB[i].is_data, ' ');
		fputs(FileDB[i].data, fd);
		fprintf(fd, "\n");
	}

	fclose(fd);
}

void dir()
{
	int filenum, dirnum;
	filenum = dirnum = 0;
	cout << "\n XZL:" << path << " ��Ŀ¼" << endl << endl;
	if (presentdir != -1)
	{
		cout << FileInfo[presentdir].day << "  " << FileInfo[presentdir].tim << "    " << "<DIR>          .\n";
		cout << FileInfo[presentdir].day << "  " << FileInfo[presentdir].tim << "    " << "<DIR>          ..\n";
	}
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].parent == presentdir)
		{
			if (FileInfo[i].type == 0)
			{
				dirnum++;
				cout << FileInfo[i].day << "  " << FileInfo[i].tim << "    " << "<DIR>          ";//2  4  10

				cout << FileInfo[i].filename << endl;
			}
			if (FileInfo[i].type == 1)
			{
				filenum++;
				cout << FileInfo[i].day << "  " << FileInfo[i].tim;
				cout.width(18);
				cout << FileInfo[i].length << " ";
				cout << FileInfo[i].filename << endl;
			}

		}
	}
	cout << "              " << filenum << " ���ļ�";
	cout.width(15);
	cout << FileInfo.size() * 57 + FileDB.size() * 256;
	cout << " �ֽ�" << endl;
	cout << "              " << dirnum << " ��Ŀ¼";
	cout.width(15);
	cout << 15048 - FileInfo.size() * 57 - FileDB.size() * 256;
	cout << " �����ֽ�" << endl;
}

void cd(string s)
{
	int flag;
	flag = 0;
	int sig = s.find_first_of("\\");
	if (sig == -1)//���뵱ǰĿ¼��Ŀ¼
	{
		if (s == "..")//�ҵ����һ��\���±꣬ɾ��\��֮����ַ�������ȻpresentdirҲҪ��
		{
			int pos;
			if (presentdir == -1)
				return;
			else
			{
				pos = path.find_last_of('\\');//�����±�
				if (pos == 0)
					path.erase(pos + 1);
				else
					path.erase(pos);
				presentdir = FileInfo[presentdir].parent;
				return;
			}
		}
		else if (s == "....")
		{
			presentdir = -1;
			path = "\\";
			return;
		}
		for (int i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir)
			{
				if (presentdir != -1)
					path.append("\\");
				presentdir = i;
				path.append(FileInfo[i].filename);
				flag = 1;
			}
		}
	}
	else
	{
		//Խ������
		stringstream ss;
		string tok;
		vector<string> dirr;//��̬
		int j = 0;
		ss.str(s);//��ʼ��
		while (getline(ss, tok, '\\'))//�ִ�
		{
			dirr.push_back(tok);
			j++;
		}
		for (int k = 0; k < j; k++)
		{
			flag = 0;
			for (int i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].parent == presentdir)
				{
					flag = 1;
					presentdir = i;
					break;
				}
			}
		}
		if (flag == 1)
		{
			for (int i = 0; i < j; i++)
			{
				if (path[path.length() - 1] != '\\')
					path.append("\\");
				path.append(dirr[i]);
			}
		}
	}
	if (flag == 0)
		cout << "ϵͳ�Ҳ���ָ����·����" << endl << endl;
}

void mkdir(string s)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s &&FileInfo[i].parent == presentdir)
		{
			cout << "��Ŀ¼���ļ� " << s << "�Ѿ����ڡ�" << endl << endl;
			return;
		}
	}
	time_t t = time(NULL);
	char ch1[20] = { 0 };
	char ch2[20] = { 0 };
	strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //��/��/��
	strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//ʱ:��
	strcpy(FileInput.filename, s.c_str());
	FileInput.parent = presentdir;
	FileInput.mode = 2;
	FileInput.length = -1;
	FileInput.addr = -1;
	FileInput.type = 0;
	strcpy(FileInput.day, ch1);
	strcpy(FileInput.tim, ch2);
	FileInfo.push_back(FileInput);
}

void rmdir(string s)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 0)
		{
			FileInfo.erase(FileInfo.begin() + i);//a.erase(a.begin()+2);ɾ��������Ԫ��
			return;
		}
	}
	cout << "ϵͳ�Ҳ���ָ�����ļ���\n��Ŀ¼������Ч��" << endl << endl;
}

void del(string s)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			FileDB.erase(FileDB.begin() + FileInfo[i].addr);//��ɾ����
			FileInfo.erase(FileInfo.begin() + i);
			return;
		}
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 0)
		{
			if (presentdir != -1)
				path.append("\\");
			while (1)
			{
				cout << "XZL:" << path << s << "\\*, �Ƿ�ȷ��(Y/N)?";
				char ch;
				ch = getchar();
				//cin.clear();
				if (ch == 'y' || ch == 'Y')
				{
					for (int j = 0; j < FileInfo.size(); j++)
					{
						if (FileInfo[j].parent == i)
						{
							if (FileInfo[j].type == 1)
								FileDB.erase(FileDB.begin() + FileInfo[j].addr);
							FileInfo.erase(FileInfo.begin() + j);
						}
					}
					cout << endl << endl;
					//cin.clear();
					cin.ignore();
					return;
				}
				if (ch == 'n' || ch == 'N')
				{
					cin.ignore();
					return;
				}
			}

		}
	}
	if (presentdir != -1)
		path.append("\\");
	cout << "�Ҳ��� XZL:" << path << s << endl << endl;
}

void find(string s, string f)
{
	int flag = 0;
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == f && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			flag = 1;
			string ss;
			int j = FileInfo[i].addr;
			while (j != -1)//��ȫ�ļ��е��ַ���
			{
				ss += FileDB[j].data;
				j = FileDB[j].next_num;
			}
			int start = ss.find_first_of(s, 0);//���ص�һ��ƥ����
			cout << endl << "---------- " << f << endl;
			if (start == -1)
				return;
			int k = start;
			while (ss[k] != ' '&& k < ss.size())
			{
				cout << ss[k];
				k++;
			}
			cout << endl << endl;
			return;
		}
	}
	if (flag == 0)
		cout << "�Ҳ����ļ� -" << f << endl << endl;
}

void show(string s)
{
	int flag = 0;
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			flag = 1;
			if (FileInfo[i].mode == 1)
			{
				cout << "���ļ�Ϊֻд�ļ������ɶ���" << endl << endl;
				return;
			}
			int j = FileInfo[i].addr;
			while (j != -1)//��ȫ�ļ��е��ַ���
			{
				cout << FileDB[j].data << endl;
				j = FileDB[j].next_num;
				
			}
			cout << endl << endl;
			return;
		}
	}
	if (flag == 0)
		cout << "ϵͳ�Ҳ���ָ�����ļ���" << endl << endl;
}

void rename(string org, string neww)
{
	int flag = 0;
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == org && FileInfo[i].parent == presentdir)//���Ը��ļ���Ҳ���Ը�Ŀ¼��
		{
			flag = 1;
			for (int j = 0; j < FileInfo.size(); j++)
			{
				if (FileInfo[j].filename == neww&&FileInfo[j].parent == presentdir)
				{
					cout << "����һ�������ļ���" << endl << endl;
					return;
				}

			}
			strcpy(FileInfo[i].filename, neww.c_str());
			cout << endl << endl;
			return;
		}
	}
	if (flag == 0)
		cout << "ϵͳ�Ҳ���ָ�����ļ���" << endl << endl;
}

void echo(string s)
{
	stringstream ss;
	string tok;
	string con[2];
	int j = 0;
	int address = 0;
	ss.str(s);//��ʼ��
	if (s.find_first_of(">") != -1)//�ȷִʣ��ٿ����Ǹ���д����׷��д
	{
		if (s.find_first_of(">") == s.find_last_of(">"))
		{
			while (getline(ss, tok, '>'))
			{
				con[j] = tok;
				//cout << tok << endl;//������
				j++;
			}
		}
		if (s.find_last_of(">") - s.find_first_of(">") == 1)
		{
			getline(ss, tok, '>');
			con[0] = tok;
			int n = s.find_last_of(">") + 1;
			int m = 0;
			while (s[n] != NULL)
			{
				con[1] += s[n];
				n++;
			}
		}

		int flag = 0;//��ǰĿ¼��������ļ���
		int i;
		for (i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == con[1] && FileInfo[i].parent == presentdir&&FileInfo[i].type == 1)
			{
				flag = 1;//�Ѵ���
				break;//���±�i�����
			}
		}
		//���ж��Ƿ�Ϊ׷��д
		if (s.find_first_of(">") == s.find_last_of(">"))//����д
		{
			if (flag == 1)
			{
				if (FileInfo[i].mode == 0)
				{
					cout << "���ļ�Ϊֻ���ļ�������д��" << endl << endl;
					return;
				}
				FileInfo[i].length = con[0].size();
				if (con[0].size() <= 256)
				{
					FileDB[FileInfo[i].addr].next_num = -1;
					strcpy(FileDB[FileInfo[i].addr].data, con[0].c_str());
					FileDB[FileInfo[i].addr].is_data = 1;
				}
				else //strcpy�ǲ���ʵ�ֵ�
				{
					int chsize = 0;
					int k = 1;//����
					FileDB[FileInfo[i].addr].next_num = FileDB.size();
					strncpy(FileDB[FileInfo[i].addr].data, con[0].c_str(), 256);
					FileDB[FileInfo[i].addr].is_data = 1;
					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);//��strncpy�����ܻ��е�Σ��
						DBInput.next_num = FileDB.size() + 1;
						DBInput.is_data = 1;
						FileDB.push_back(DBInput);
						chsize += 256;
						k++;
					}
					strncpy(DBInput.data, con[0].c_str() + 256 * k, con[0].size() - chsize);
					DBInput.next_num = -1;
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
			}
			else  //�ļ�������
			{
				address = FileDB.size();
				if (con[0].size() <= 256)
				{
					DBInput.next_num = -1;
					strcpy(DBInput.data, con[0].c_str());
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
				else
				{
					int chsize = 0;
					int k = 1;//����
					DBInput.next_num = FileDB.size();
					strncpy(DBInput.data, con[0].c_str(), 256);
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);

					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);
						DBInput.next_num = FileDB.size() + 1;
						DBInput.is_data = 1;
						FileDB.push_back(DBInput);
						chsize += 256;
						k++;
					}
					strncpy(DBInput.data, con[0].c_str() + 256 * k, con[0].size() - chsize);
					DBInput.next_num = -1;
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
			}
		}
		else  //׷��д
		{
			if (flag == 1)
			{
				FileInfo[i].length += con[0].size();
				int ii = FileInfo[i].addr;
				int iii = ii;
				while (iii != -1)
				{
					iii = FileDB[ii].next_num;
					if (iii != -1)
						ii = iii;
				}  //����ѭ��ʱiii=-1
				if (con[0].size() <= 256)
				{
					FileDB[ii].next_num = FileDB.size();
					DBInput.next_num = -1;
					strcpy(DBInput.data, con[0].c_str());
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
				else //strcpy�ǲ���ʵ�ֵ�
				{
					int chsize = 0;
					int k = 1;//����
					FileDB[FileInfo[ii].addr].next_num = FileDB.size();
					strncpy(FileDB[FileInfo[ii].addr].data, con[0].c_str(), 256);
					FileDB[FileInfo[ii].addr].is_data = 1;
					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);//��strncpy
						DBInput.next_num = FileDB.size() + 1;
						DBInput.is_data = 1;
						FileDB.push_back(DBInput);
						chsize += 256;
						k++;
					}
					strncpy(DBInput.data, con[0].c_str() + 256 * k, con[0].size() - chsize);
					DBInput.next_num = -1;
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
			}
			else  //�ļ�������
			{
				address = FileDB.size();
				if (con[0].size() <= 256)
				{
					DBInput.next_num = -1;
					strcpy(DBInput.data, con[0].c_str());
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
				else
				{
					int chsize = 0;
					int k = 1;//����
					DBInput.next_num = FileDB.size();
					strncpy(DBInput.data, con[0].c_str(), 256);
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);

					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);
						DBInput.next_num = FileDB.size() + 1;
						DBInput.is_data = 1;
						FileDB.push_back(DBInput);
						chsize += 256;
						k++;
					}
					strncpy(DBInput.data, con[0].c_str() + 256 * k, con[0].size() - chsize);
					DBInput.next_num = -1;
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
			}
		}

		if (flag == 0)//�����ڣ��½�һ��
		{
			time_t t = time(NULL);
			char ch1[20] = { 0 };
			char ch2[20] = { 0 };
			strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //��/��/��
			strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//ʱ:��
			//��ַ����������Ҫ��
			strcpy(FileInput.filename, con[1].c_str());
			FileInput.type = 1;
			FileInput.length = con[0].size();
			FileInput.addr = address;
			FileInput.parent = presentdir;
			FileInput.mode = 2;
			strcpy(FileInput.day, ch1);
			strcpy(FileInput.tim, ch2);
			FileInfo.push_back(FileInput);
		}
	}
	else
		cout << "�����﷨����ȷ��" << endl << endl;
}

void import(string p)
{
	FILE *fd;
	if ((fd = fopen(p.c_str(), "r")) == NULL)
	{
		cout << "�����ļ�����ʧ�ܣ�" << endl;
		return;
	}
	stringstream ss;
	string tok;
	vector<string> dirr;//��̬
	int j = 0;
	ss.str(p);//��ʼ��
	while (getline(ss, tok, '\\'))//�ִ�
	{
		dirr.push_back(tok);
		j++;
	}

	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == dirr[j - 1] && FileInfo[i].parent == presentdir)//��ֹ����
		{
			cout << "����һ�������ļ���" << endl << endl;
			return;
		}
	}

	time_t t = time(NULL);
	char ch1[20] = { 0 };
	char ch2[20] = { 0 };
	strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //��/��/��
	strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//ʱ:��
	strcpy(FileInput.filename, dirr[j - 1].c_str());
	FileInput.type = 1;
	FileInput.addr = FileDB.size();
	FileInput.parent = presentdir;
	FileInput.mode = 2;
	strcpy(FileInput.day, ch1);
	strcpy(FileInput.tim, ch2);
	char Tempbuf[256];
	int chsize = -256;
	while (fgets(Tempbuf, 256, fd) != NULL)
	{
		fgets(Tempbuf, 256, fd);
		strcpy(DBInput.data, Tempbuf);
		DBInput.is_data = 1;
		DBInput.next_num = FileDB.size();
		FileDB.push_back(DBInput);
		chsize += 256;
		cout << DBInput.data;
	}
	chsize += strlen(DBInput.data);
	FileDB[FileDB.size() - 1].next_num = -1;
	FileInput.length = chsize - 1;
	FileInfo.push_back(FileInput);

	fclose(fd);
}

void my_export(string s, string p)
{
	int flag = 0;
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			flag = 1;
			string ss;
			int j = FileInfo[i].addr;
			while (j != -1)//��ȫ�ļ��е��ַ���
			{
				ss += FileDB[j].data;
				j = FileDB[j].next_num;
			}
			p += s;//ƴ��·��
			FILE* fd;
			if ((fd = fopen(p.c_str(), "w")) == NULL)
			{
				cout << "����ʧ�ܣ�" << endl;
				return;
			}
			if (ss[0] == ' ')
				ss.erase(0, 1);
			fputs(ss.c_str(), fd);
			fclose(fd);
		}
	}
	if (flag == 0)
		cout << "ϵͳ�Ҳ���ָ�����ļ���" << endl << endl;
}

void move(string org, string neww)
{
	int flag1, flag2;
	flag1 = flag2 = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;//��̬
	int j = 0;
	int fina;//���ո�Ŀ¼
	int begi;//Ҫ�ƶ����±�
	ss.str(neww);//��ʼ��
	while (getline(ss, tok, '\\'))//�ִ�
	{
		dirr.push_back(tok);
		//cout << tok << endl;//������
		j++;
	}
	if (dirr[j - 1] == ".")
	{
		fina = -1;
		flag1 = 1;
	}
	else
	{
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (int i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0)
				{
					flag1 = 1;
					fina = i;
					break;
				}
			}
		}
	}
	

	int sig = org.find_first_of("\\");
	if (sig == -1)//Ҫ�ƶ��Ĳ���·����
	{
		for (int i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == org && FileInfo[i].parent == presentdir)
			{
				flag2 = 1;
				begi = i;
			}
		}
	}
	else
	{
		stringstream sss;
		string tokk;
		vector<string> dirrr;//��̬
		int j = 0;
		sss.str(org);//��ʼ��
		while (getline(sss, tokk, '\\'))//�ִ�
		{
			dirr.push_back(tokk);
			//cout << tokk << endl;//������
			j++;
		}
		for (int k = 0; k < j; k++)
		{
			flag2 = 0;
			for (int i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k])
				{
					flag2 = 1;
					begi = i;
					break;
				}
			}
		}
	}

	if (flag1 == 1 && flag2 == 1)//����·����ȷ
	{
		FileInfo[begi].parent = fina;
	}
	else
		cout << "ϵͳ�Ҳ���ָ����·����" << endl << endl;
}

void attrib(string t, string s)
{
	int flag, i;
	flag = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;
	int j = 0;
	ss.str(s);//��ʼ��
	while (getline(ss, tok, '\\'))//�ִ�
	{
		dirr.push_back(tok);
		j++;
	}
	for (int k = 0; k < j; k++)
	{
		flag = 0;
		for (i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == dirr[k])
			{
				flag = 1;
				break;
			}
		}
	}

	if (flag == 1)
	{
		if (t == "r")
			FileInfo[i].mode = 0;
		else if (t == "w")
			FileInfo[i].mode = 1;
		else if (t == "rw" || t == "wr")
			FileInfo[i].mode = 2;
	}
	else
		cout << "ϵͳ�Ҳ���ָ�����ļ���" << endl << endl;
}

void copy(string f, string p)
{
	int flag1;
	int flag2, i;
	int par = 0;
	flag1 = flag2 = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;
	int j = 0;
	ss.str(p);//��ʼ��
	while (getline(ss, tok, '\\'))//�ִ�
	{
		dirr.push_back(tok);
		//cout << tok << endl;//������
		j++;
	}
	if (dirr[j-1] == ".")
	{
		par = -1;
		flag1 = 1;
	}
	else
	{
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0)
				{
					flag1 = 1;
					par = i;
					break;
				}
			}
		}
	}
	if (flag1 == 1)
	{
		for (int i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == f && FileInfo[i].type == 1)
			{
				flag2 = 1;
				strcpy(FileInput.filename, f.c_str());
				FileInput.length = FileInfo[i].length;
				FileInput.mode = FileInfo[i].mode;
				FileInput.type = FileInfo[i].type;
				strcpy(FileInput.day, FileInfo[i].day);
				strcpy(FileInput.tim, FileInfo[i].tim);
				FileInput.parent = par;
				FileInput.addr = FileDB.size();
				int ii = FileInfo[i].addr;
				int iii = ii;
				while (iii != -1)
				{
					iii = FileDB[ii].next_num;
					strcpy(DBInput.data, FileDB[ii].data);
					DBInput.is_data = 1;
					if (iii != -1)
						DBInput.next_num = FileDB.size();
					else
						DBInput.next_num = -1;
					FileDB.push_back(DBInput);

				}  //����ѭ��ʱiii=-1
				FileInfo.push_back(FileInput);
				cout << endl << endl;
				return;
			}
		}
	}

	if (flag1 == 0 || flag2 == 0)
		cout << "ϵͳ�Ҳ���ָ�����ļ���·����" << endl << endl;
}

void copy2(int i, int par)//�ļ�����
{
	strcpy(FileInput.filename, FileInfo[i].filename);
	FileInput.length = FileInfo[i].length;
	FileInput.mode = FileInfo[i].mode;
	FileInput.type = FileInfo[i].type;
	strcpy(FileInput.day, FileInfo[i].day);
	strcpy(FileInput.tim, FileInfo[i].tim);
	FileInput.parent = par;
	FileInput.addr = FileDB.size();
	int ii = FileInfo[i].addr;
	int iii = ii;
	while (iii != -1)
	{
		iii = FileDB[ii].next_num;
		strcpy(DBInput.data, FileDB[ii].data);
		DBInput.is_data = 1;
		if (iii != -1)
			DBInput.next_num = FileDB.size();
		else
			DBInput.next_num = -1;
		FileDB.push_back(DBInput);

	}  //����ѭ��ʱiii=-1
	FileInfo.push_back(FileInput);
	cout << endl << endl;
}
void copy3(int i,int par)//Ŀ¼����
{
	strcpy(FileInput.filename, FileInfo[i].filename);
	FileInput.length = -1;
	FileInput.mode = FileInfo[i].mode;
	FileInput.type = FileInfo[i].type;
	strcpy(FileInput.day, FileInfo[i].day);
	strcpy(FileInput.tim, FileInfo[i].tim);
	FileInput.parent = par;
	FileInput.addr = -1;
	FileInfo.push_back(FileInput);
}

void copy4(int i,int par)//�ݹ�
{
	if (FileInfo[i].type == 1)
		copy2(i,par);
	else
	{
		copy3(i, par);
		for (int k = 0; k < FileInfo.size(); k++)
		{
			if (FileInfo[k].parent == i)
			{
					copy4(k, FileInfo.size()-1);
			}
		}
	}
}

void xcopy(string f, string p)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == f && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			copy(f, p);
			return;
		}
	}

	int flag1;
	int flag2, i;
	int par = 0;
	flag1 = flag2 = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;
	int j = 0;
	ss.str(p);//��ʼ��
	while (getline(ss, tok, '\\'))//�ִ�
	{
		dirr.push_back(tok);
		j++;
	}
	if (dirr[j - 1] == ".")
	{
		par = -1;
		flag1 = 1;
	}
	else
	{
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0)
				{
					flag1 = 1;
					par = i;
					break;
				}
			}
		}
	}
	if (flag1 == 1)
	{
		for (int i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == f && FileInfo[i].type == 0)//����Ŀ¼��
			{
				
				flag2 = 1;
				copy4(i, par);
				return;
			}
		}
	}

	if (flag1 == 0 || flag2 == 0)
		cout << "ϵͳ�Ҳ���ָ�����ļ���·����" << endl << endl;
}

int main()
{
	SetWindowTextA(GetConsoleWindow(), "XZL CMD.exe");
	int i;
	cout << "XZL COMMAND [�汾 2.0.1]";
	cout << endl;
	cout << "XZL:" << path << ">";
	string strr;
	string cl[4];//command line
	string tok;
	stringstream str;
	getline(cin, strr);//����ֱ����cin
	while (strr != "exit")
	{
		str.str(strr);
		i = 0;
		while (getline(str, tok, ' '))//���ո�ִ�
		{
			cl[i] = tok;
			//cout << tok << endl;
			i++;
		}
		if (cl[0] == "dir")//�������Ҫ�����ִ��ˣ��Ѿ����presentdir
		{
			initFiletoRom();
			dir();
			cout << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "cd")//Խ������
		{
			if (cl[1] != "")
			{
				initFiletoRom();
				cd(cl[1]);
				cout << endl << endl;
				out_to_file();
			}
				
		}
		else if (cl[0] == "mkdir")
		{
			if (cl[1] != "")
			{
				initFiletoRom();
				mkdir(cl[1]);
				cout << endl << endl;
				out_to_file();
			}
			else
				cout << "�����﷨����ȷ��" << endl << endl;
		}
		else if (cl[0] == "rmdir")  //        ����������ʼ��Ҫ�ǵ����ļ�����ʱ��ͬʱ��Ҫ�Ƚϵ�ǰĿ¼����������������
		{
			if (cl[1] != "")
			{
				initFiletoRom();
				rmdir(cl[1]);
				out_to_file();
			}
				
			else
				cout << "�����﷨����ȷ��" << endl << endl;
		}
		else if (cl[0] == "del")
		{
			initFiletoRom();
			if (cl[1] != "")
				del(cl[1]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "find")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				find(cl[1], cl[2]);
			else
				cout << "FIND: ������ʽ����ȷ" << endl << endl;
			out_to_file();

		}
		else if (cl[0] == "cls")//���������д������
		{
			system("cls");
		}
		else if (cl[0] == "date")
		{
			time_t t = time(NULL);
			char ch[64] = { 0 };
			strftime(ch, sizeof(ch) - 1, "%Y/%m/%d %A", localtime(&t));     //��/��/�� �ܼ�
			cout << "��ǰ����: " << ch << endl << endl;
		}
		else if (cl[0] == "time")
		{
			time_t t = time(NULL);
			char ch[64] = { 0 };
			strftime(ch, sizeof(ch) - 1, "%T", localtime(&t));     //ʱ/��/��/����
			cout << "��ǰʱ��: " << ch << endl << endl;
		}
		else if (cl[0] == "format")
		{
			initFiletoRom();
			FileInfo.clear();
			FileDB.clear();
			cout << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "echo")
		{
			initFiletoRom();
			if (cl[1] != "")
			{
				echo(cl[1]);
				cout << endl << endl;
			}
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "move")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
			{
				move(cl[1], cl[2]);
				cout << endl << endl;
			}
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "import")
		{
			initFiletoRom();
			if (cl[1] != "")
				import(cl[1]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "export")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				my_export(cl[1], cl[2]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "copy")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				copy(cl[1], cl[2]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "xcopy")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				xcopy(cl[1], cl[2]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "type" || cl[0] == "more")
		{
			initFiletoRom();
			if (cl[1] != "")
				show(cl[1]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "rename")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				rename(cl[1], cl[2]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "attrib")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				attrib(cl[1], cl[2]);
			else
				cout << "�����﷨����ȷ��" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "help" && cl[1] != "")
		{
			if (cl[1] == "rename")
				cout << "RENAME OriginName NewName" << endl << endl;
			if (cl[1] == "find")
				cout << "FIND string filename" << endl << endl;
			if (cl[0] == "echo")
			{
				cout << "ECHO string>filename" << endl << endl;
				cout << "ECHO string>>filename" << endl << endl;
			}
			if (cl[0] == "attrib")
				cout << "ATTRIB r/w/rw filename" << endl << endl;
		}
		else if (cl[0] == "help"&& cl[1] == "")
		{
			cout << "ATTRIB         ��ʾ������ļ����ԡ�" << endl;
			cout << "CD             ����Ŀ¼�򷵻���һ����" << endl;
			cout << "CLS            �����Ļ��" << endl;
			cout << "COPY           ��һ���ļ����Ƶ���һ��λ�á�" << endl;
			cout << "DATE           ��ʾ���ڡ�" << endl;
			cout << "DEL            ɾ��һ���ļ���Ŀ¼���������ݡ�" << endl;
			cout << "DIR            ��ʾһ��Ŀ¼�е��ļ�����Ŀ¼��" << endl;
			cout << "ECHO           ��ʾ��Ϣ����������Դ򿪻�رա�" << endl;
			cout << "EXIT           �˳� XZL CMD.EXE ����(������ͳ���)��" << endl;
			cout << "EXPORT			����ǰĿ¼�µ��ļ����������ش��̡�" << endl;
			cout << "FIND           ��һ���ļ�������һ���ı��ַ�����" << endl;
			cout << "FORMAT         ��ʽ�����̡�" << endl;
			cout << "IMPORT			�ӱ��ش��̸������ݵ���ǰĿ¼��" << endl;
			cout << "HELP           �ṩ����İ�����Ϣ��" << endl;
			cout << "MKDIR          ����һ��Ŀ¼��" << endl;
			cout << "MORE           ������ʾ�����" << endl;
			cout << "MOVE           ��һ���ļ���һ��Ŀ¼�ƶ�����һ��Ŀ¼��" << endl;
			cout << "RENAME         �������ļ���Ŀ¼��" << endl;//Ҫ������������
			cout << "RMDIR          ɾ��Ŀ¼��" << endl;
			cout << "TIME           ��ʾϵͳʱ�䡣" << endl;
			cout << "TYPE           ��ʾ�ı��ļ������ݡ�" << endl;
			cout << "XCOPY          �����ļ���Ŀ¼����" << endl;
		}
		else
		{
			cout << "'" << cl[0] << "' �����ڲ����ⲿ���Ҳ���ǿ����еĳ���\n���������ļ���" << endl << endl;
		}

		cout << "XZL:" << path << ">";
		cin.clear();
		getline(cin, strr);
		cin.clear();

		str.clear();
		str.str("");
		cl[0] = "";
		cl[1] = "";
		cl[2] = "";
		cl[3] = "";
	}
	out_to_file();
	exit(0);
	//system("pause");
	return 0;
}

