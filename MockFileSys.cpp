// MockFileSys.cpp : 定义控制台应用程序的入口点。
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
	int type;//0目录，1文件
	int mode; // 文件权限0 - readonly;  1 - writeonly;  2 - read / write
	int length;//文件长度(以字节数计算)  
	int addr;//如果为文件，则是DB下标；如果为文件夹，无意义-1
	int parent;//上一级目录，根目录-1，文件-1
	char day[11];//多一个\0
	char tim[6];
}UFD;  //文件或目录   57B


typedef struct
{
	int next_num;//下一个块号（可能不连续），没有下一块-1
	int is_data;//启用还是不启用
	char data[256];

}DB;  //数据块  一维数组   264B

//声明
extern int Headnum;
extern int Middlenum;
extern int presentdir;//当前目录
extern UFD FileInput;
extern DB DBInput;
extern vector <UFD> FileInfo;
extern vector<DB> FileDB;
extern string path;

//定义
int Headnum;
int Middlenum;
int presentdir = -1;//从根目录开始
UFD FileInput;
DB DBInput;
vector <UFD> FileInfo;  //一维动态数组
vector<DB> FileDB;
string path = "\\";



void initFiletoRom()
{
	FileInfo.clear();
	FileDB.clear();
	FILE *fd;
	if ((fd = fopen("disk.txt", "r")) == NULL)
	{
		cout << "磁盘中读入失败！" << endl;
		return;
	}

	fscanf(fd, "%d", &Headnum);
	int alreadynum;
	int ret;
	alreadynum = 0;

	//初始化文件信息
	for (int i = 0; i < Headnum; i++)
	{
		if ((ret = fscanf(fd, "%s %d %d %d %d %d %s %s", &FileInput.filename, &FileInput.type, &FileInput.mode, &FileInput.length, &FileInput.addr, &FileInput.parent, &FileInput.day, &FileInput.tim)) != -1)
		{
			FileInfo.push_back(FileInput);
			//cout << FileInput.filename << FileInput.type << FileInput.mode << FileInput.length << FileInput.addr << FileInput.parent << FileInput.day << " " << FileInput.tim << endl;
		}

	}
	alreadynum = 0;
	char Tempbuf[256];  //一个文件块最多256
	char c;
	fscanf(fd, "%d", &Middlenum);
	while (alreadynum < Middlenum)
	{
		memset(Tempbuf, 0, sizeof(Tempbuf));//全部初始化0
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

void out_to_file()  //覆盖写
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
	cout << "\n XZL:" << path << " 的目录" << endl << endl;
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
	cout << "              " << filenum << " 个文件";
	cout.width(15);
	cout << FileInfo.size() * 57 + FileDB.size() * 256;
	cout << " 字节" << endl;
	cout << "              " << dirnum << " 个目录";
	cout.width(15);
	cout << 15048 - FileInfo.size() * 57 - FileDB.size() * 256;
	cout << " 可用字节" << endl;
}

void cd(string s)
{
	int flag;
	flag = 0;
	int sig = s.find_first_of("\\");
	if (sig == -1)//进入当前目录下目录
	{
		if (s == "..")//找到最后一个\的下标，删除\及之后的字符串，当然presentdir也要改
		{
			int pos;
			if (presentdir == -1)
				return;
			else
			{
				pos = path.find_last_of('\\');//返回下标
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
		//越级访问
		stringstream ss;
		string tok;
		vector<string> dirr;//动态
		int j = 0;
		ss.str(s);//初始化
		while (getline(ss, tok, '\\'))//分词
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
		cout << "系统找不到指定的路径。" << endl << endl;
}

void mkdir(string s)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s &&FileInfo[i].parent == presentdir)
		{
			cout << "子目录或文件 " << s << "已经存在。" << endl << endl;
			return;
		}
	}
	time_t t = time(NULL);
	char ch1[20] = { 0 };
	char ch2[20] = { 0 };
	strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //年/月/日
	strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//时:分
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
			FileInfo.erase(FileInfo.begin() + i);//a.erase(a.begin()+2);删除第三个元素
			return;
		}
	}
	cout << "系统找不到指定的文件。\n或目录名称无效。" << endl << endl;
}

void del(string s)
{
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 1)
		{
			FileDB.erase(FileDB.begin() + FileInfo[i].addr);//先删数据
			FileInfo.erase(FileInfo.begin() + i);
			return;
		}
		if (FileInfo[i].filename == s && FileInfo[i].parent == presentdir && FileInfo[i].type == 0)
		{
			if (presentdir != -1)
				path.append("\\");
			while (1)
			{
				cout << "XZL:" << path << s << "\\*, 是否确认(Y/N)?";
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
	cout << "找不到 XZL:" << path << s << endl << endl;
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
			while (j != -1)//找全文件中的字符串
			{
				ss += FileDB[j].data;
				j = FileDB[j].next_num;
			}
			int start = ss.find_first_of(s, 0);//返回第一次匹配结果
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
		cout << "找不到文件 -" << f << endl << endl;
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
				cout << "该文件为只写文件，不可读。" << endl << endl;
				return;
			}
			int j = FileInfo[i].addr;
			while (j != -1)//找全文件中的字符串
			{
				cout << FileDB[j].data << endl;
				j = FileDB[j].next_num;
				
			}
			cout << endl << endl;
			return;
		}
	}
	if (flag == 0)
		cout << "系统找不到指定的文件。" << endl << endl;
}

void rename(string org, string neww)
{
	int flag = 0;
	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == org && FileInfo[i].parent == presentdir)//可以改文件名也可以改目录名
		{
			flag = 1;
			for (int j = 0; j < FileInfo.size(); j++)
			{
				if (FileInfo[j].filename == neww&&FileInfo[j].parent == presentdir)
				{
					cout << "存在一个重名文件。" << endl << endl;
					return;
				}

			}
			strcpy(FileInfo[i].filename, neww.c_str());
			cout << endl << endl;
			return;
		}
	}
	if (flag == 0)
		cout << "系统找不到指定的文件。" << endl << endl;
}

void echo(string s)
{
	stringstream ss;
	string tok;
	string con[2];
	int j = 0;
	int address = 0;
	ss.str(s);//初始化
	if (s.find_first_of(">") != -1)//先分词，再考虑是覆盖写还是追加写
	{
		if (s.find_first_of(">") == s.find_last_of(">"))
		{
			while (getline(ss, tok, '>'))
			{
				con[j] = tok;
				//cout << tok << endl;//测试用
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

		int flag = 0;//当前目录下有这个文件吗
		int i;
		for (i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == con[1] && FileInfo[i].parent == presentdir&&FileInfo[i].type == 1)
			{
				flag = 1;//已存在
				break;//把下标i提出来
			}
		}
		//先判断是否为追加写
		if (s.find_first_of(">") == s.find_last_of(">"))//覆盖写
		{
			if (flag == 1)
			{
				if (FileInfo[i].mode == 0)
				{
					cout << "该文件为只读文件，不可写。" << endl << endl;
					return;
				}
				FileInfo[i].length = con[0].size();
				if (con[0].size() <= 256)
				{
					FileDB[FileInfo[i].addr].next_num = -1;
					strcpy(FileDB[FileInfo[i].addr].data, con[0].c_str());
					FileDB[FileInfo[i].addr].is_data = 1;
				}
				else //strcpy是不能实现的
				{
					int chsize = 0;
					int k = 1;//倍数
					FileDB[FileInfo[i].addr].next_num = FileDB.size();
					strncpy(FileDB[FileInfo[i].addr].data, con[0].c_str(), 256);
					FileDB[FileInfo[i].addr].is_data = 1;
					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);//用strncpy，可能会有点危险
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
			else  //文件不存在
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
					int k = 1;//倍数
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
		else  //追加写
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
				}  //跳出循环时iii=-1
				if (con[0].size() <= 256)
				{
					FileDB[ii].next_num = FileDB.size();
					DBInput.next_num = -1;
					strcpy(DBInput.data, con[0].c_str());
					DBInput.is_data = 1;
					FileDB.push_back(DBInput);
				}
				else //strcpy是不能实现的
				{
					int chsize = 0;
					int k = 1;//倍数
					FileDB[FileInfo[ii].addr].next_num = FileDB.size();
					strncpy(FileDB[FileInfo[ii].addr].data, con[0].c_str(), 256);
					FileDB[FileInfo[ii].addr].is_data = 1;
					chsize = 256;
					while (con[0].size() - chsize > 256)
					{
						strncpy(DBInput.data, con[0].c_str() + 256 * k, 256);//用strncpy
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
			else  //文件不存在
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
					int k = 1;//倍数
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

		if (flag == 0)//不存在，新建一个
		{
			time_t t = time(NULL);
			char ch1[20] = { 0 };
			char ch2[20] = { 0 };
			strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //年/月/日
			strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//时:分
			//地址长度是最重要的
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
		cout << "命令语法不正确。" << endl << endl;
}

void import(string p)
{
	FILE *fd;
	if ((fd = fopen(p.c_str(), "r")) == NULL)
	{
		cout << "本地文件导入失败！" << endl;
		return;
	}
	stringstream ss;
	string tok;
	vector<string> dirr;//动态
	int j = 0;
	ss.str(p);//初始化
	while (getline(ss, tok, '\\'))//分词
	{
		dirr.push_back(tok);
		j++;
	}

	for (int i = 0; i < FileInfo.size(); i++)
	{
		if (FileInfo[i].filename == dirr[j - 1] && FileInfo[i].parent == presentdir)//防止重名
		{
			cout << "存在一个重名文件。" << endl << endl;
			return;
		}
	}

	time_t t = time(NULL);
	char ch1[20] = { 0 };
	char ch2[20] = { 0 };
	strftime(ch1, sizeof(ch1) - 1, "%Y/%m/%d", localtime(&t));     //年/月/日
	strftime(ch2, sizeof(ch2) - 1, "%H:%M", localtime(&t));//时:分
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
			while (j != -1)//找全文件中的字符串
			{
				ss += FileDB[j].data;
				j = FileDB[j].next_num;
			}
			p += s;//拼接路径
			FILE* fd;
			if ((fd = fopen(p.c_str(), "w")) == NULL)
			{
				cout << "导出失败！" << endl;
				return;
			}
			if (ss[0] == ' ')
				ss.erase(0, 1);
			fputs(ss.c_str(), fd);
			fclose(fd);
		}
	}
	if (flag == 0)
		cout << "系统找不到指定的文件。" << endl << endl;
}

void move(string org, string neww)
{
	int flag1, flag2;
	flag1 = flag2 = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;//动态
	int j = 0;
	int fina;//最终父目录
	int begi;//要移动的下标
	ss.str(neww);//初始化
	while (getline(ss, tok, '\\'))//分词
	{
		dirr.push_back(tok);
		//cout << tok << endl;//测试用
		j++;
	}
	if (dirr[j - 1] == ".")
	{
		fina = -1;
		flag1 = 1;
	}
	else
	{
		int re=-1;
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (int i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0 && FileInfo[i].parent==re)
				{
					flag1 = 1;
					fina = i;
					re = i;
					break;
				}
			}
		}
	}
	

	int sig = org.find_first_of("\\");
	if (sig == -1)//要移动的不是路径名
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
		vector<string> dirrr;//动态
		int j = 0;
		sss.str(org);//初始化
		while (getline(sss, tokk, '\\'))//分词
		{
			dirrr.push_back(tokk);
			//cout << tokk << endl;//测试用
			j++;
		}
		int re = -1;
		for (int k = 0; k < j; k++)
		{
			flag2 = 0;
			for (int i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirrr[k] && FileInfo[i].parent==re)
				{
					flag2 = 1;
					begi = i;
					re = i;
					break;
				}
			}
		}
	}

	if (flag1 == 1 && flag2 == 1)//两者路径正确
	{
		FileInfo[begi].parent = fina;
	}
	else
		cout << "系统找不到指定的路径。" << endl << endl;
}

void attrib(string t, string s)
{
		int flag, i;
	flag = 0;
	stringstream ss;
	string tok;
	vector<string> dirr;
	int j = 0;
	if (s == "")
		ss.str(t);//显示属性
	else
		ss.str(s);//初始化
	while (getline(ss, tok, '\\'))//分词
	{
		dirr.push_back(tok);
		j++;
	}
	int re = -1;
	for (int k = 0; k < j; k++)
	{
		flag = 0;
		for (i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == dirr[k]&& FileInfo[i].parent==re)
			{
				flag = 1;
				re = i;
				break;
			}
		}
	}

	if (flag == 1)
	{
		if (s == "")
		{
			if (FileInfo[i].mode == 0)
				cout << "Read Only";
			if (FileInfo[i].mode == 1)
				cout << "Write Only";
			if (FileInfo[i].mode == 2)
				cout << "Read & Write";
		}
		else
		{
			if (t == "r")
				FileInfo[i].mode = 0;
			else if (t == "w")
				FileInfo[i].mode = 1;
			else if (t == "rw" || t == "wr")
				FileInfo[i].mode = 2;
		}
		cout << endl << endl;
	}
	else
		cout << "系统找不到指定的文件。" << endl << endl;
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
	ss.str(p);//初始化
	while (getline(ss, tok, '\\'))//分词
	{
		dirr.push_back(tok);
		//cout << tok << endl;//测试用
		j++;
	}
	if (dirr[j-1] == ".")
	{
		par = -1;
		flag1 = 1;
	}
	else
	{
		int re = -1;
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0 && FileInfo[i].parent==re)
				{
					flag1 = 1;
					par = i;
					re = i;
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

				}  //跳出循环时iii=-1
				FileInfo.push_back(FileInput);
				cout << endl << endl;
				return;
			}
		}
	}

	if (flag1 == 0 || flag2 == 0)
		cout << "系统找不到指定的文件或路径。" << endl << endl;
}

void copy2(int i, int par)//文件复制
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

	}  //跳出循环时iii=-1
	FileInfo.push_back(FileInput);
	cout << endl << endl;
}
void copy3(int i,int par)//目录复制
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

void copy4(int i,int par)//递归
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
	ss.str(p);//初始化
	while (getline(ss, tok, '\\'))//分词
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
		int re = -1;
		for (int k = 0; k < j; k++)
		{
			flag1 = 0;
			for (i = 0; i < FileInfo.size(); i++)
			{
				if (FileInfo[i].filename == dirr[k] && FileInfo[i].type == 0 && FileInfo[i].parent==re)
				{
					flag1 = 1;
					par = i;
					re = i;
					break;
				}
			}
		}
	}
	if (flag1 == 1)
	{
		for (int i = 0; i < FileInfo.size(); i++)
		{
			if (FileInfo[i].filename == f && FileInfo[i].type == 0)//复制目录树
			{
				
				flag2 = 1;
				copy4(i, par);
				return;
			}
		}
	}

	if (flag1 == 0 || flag2 == 0)
		cout << "系统找不到指定的文件或路径。" << endl << endl;
}

int main()
{
	SetWindowTextA(GetConsoleWindow(), "XZL CMD.exe");
	int i;
	cout << "XZL COMMAND [版本 2.0.1]";
	cout << endl;
	cout << "XZL:" << path << ">";
	string strr;
	string cl[4];//command line
	string tok;
	stringstream str;
	getline(cin, strr);//不能直接用cin
	while (strr != "exit")
	{
		str.str(strr);
		i = 0;
		while (getline(str, tok, ' '))//按空格分词
		{
			cl[i] = tok;
			//cout << tok << endl;
			i++;
		}
		if (cl[0] == "dir")//这个不需要再做分词了，已经获得presentdir
		{
			initFiletoRom();
			dir();
			cout << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "cd")//越级访问
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
				cout << "命令语法不正确。" << endl << endl;
		}
		else if (cl[0] == "rmdir")  //        【【【【【始终要记得找文件名的时候同时还要比较当前目录】】】】】】】】
		{
			if (cl[1] != "")
			{
				initFiletoRom();
				rmdir(cl[1]);
				out_to_file();
			}
				
			else
				cout << "命令语法不正确。" << endl << endl;
		}
		else if (cl[0] == "del")
		{
			initFiletoRom();
			if (cl[1] != "")
				del(cl[1]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "find")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				find(cl[1], cl[2]);
			else
				cout << "FIND: 参数格式不正确" << endl << endl;
			out_to_file();

		}
		else if (cl[0] == "cls")//这个不单独写函数了
		{
			system("cls");
		}
		else if (cl[0] == "date")
		{
			time_t t = time(NULL);
			char ch[64] = { 0 };
			strftime(ch, sizeof(ch) - 1, "%Y/%m/%d %A", localtime(&t));     //年/月/日 周几
			cout << "当前日期: " << ch << endl << endl;
		}
		else if (cl[0] == "time")
		{
			time_t t = time(NULL);
			char ch[64] = { 0 };
			strftime(ch, sizeof(ch) - 1, "%T", localtime(&t));     //时/分/秒/分秒
			cout << "当前时间: " << ch << endl << endl;
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
				cout << "命令语法不正确。" << endl << endl;
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
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "import")
		{
			initFiletoRom();
			if (cl[1] != "")
				import(cl[1]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "export")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				my_export(cl[1], cl[2]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "copy")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				copy(cl[1], cl[2]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "xcopy")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				xcopy(cl[1], cl[2]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "type" || cl[0] == "more")
		{
			initFiletoRom();
			if (cl[1] != "")
				show(cl[1]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "rename")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				rename(cl[1], cl[2]);
			else
				cout << "命令语法不正确。" << endl << endl;
			out_to_file();
		}
		else if (cl[0] == "attrib")
		{
			initFiletoRom();
			if (cl[1] != "" && cl[2] != "")
				attrib(cl[1], cl[2]);
			else
				cout << "命令语法不正确。" << endl << endl;
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
			cout << "ATTRIB         更改文件属性。" << endl;
			cout << "CD             进入目录或返回上一级。" << endl;
			cout << "CLS            清除屏幕。" << endl;
			cout << "COPY           将一个文件复制到另一个位置。" << endl;
			cout << "DATE           显示日期。" << endl;
			cout << "DEL            删除一个文件或目录下所有内容。" << endl;
			cout << "DIR            显示一个目录中的文件和子目录。" << endl;
			cout << "ECHO           显示消息，或将命令回显打开或关闭。" << endl;
			cout << "EXIT           退出 XZL CMD.EXE 程序(命令解释程序)。" << endl;
			cout << "EXPORT			将当前目录下的文件导出到本地磁盘。" << endl;
			cout << "FIND           在一个文件中搜索一个文本字符串。" << endl;
			cout << "FORMAT         格式化磁盘。" << endl;
			cout << "IMPORT			从本地磁盘复制内容到当前目录。" << endl;
			cout << "HELP           提供命令的帮助信息。" << endl;
			cout << "MKDIR          创建一个目录。" << endl;
			cout << "MORE           逐屏显示输出。" << endl;
			cout << "MOVE           将一个文件从一个目录移动到另一个目录。" << endl;
			cout << "RENAME         重命名文件或目录。" << endl;//要接受两个参数
			cout << "RMDIR          删除目录。" << endl;
			cout << "TIME           显示系统时间。" << endl;
			cout << "TYPE           显示文本文件的内容。" << endl;
			cout << "XCOPY          复制文件和目录树。" << endl;
		}
		else
		{
			cout << "'" << cl[0] << "' 不是内部或外部命令，也不是可运行的程序\n或批处理文件。" << endl << endl;
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

