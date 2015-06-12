#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <list>
using namespace std;

typedef vector< pair< string, bool > > myStruct;

struct Implicant
{
	myStruct* Group;
	
	Implicant(int NumOfVar)
	{
		this->Group = new myStruct[NumOfVar];
	}
	Implicant() :Group(nullptr){}
	~Implicant()
	{
		if (this->Group != nullptr) {
			delete[] Group;
		}
	}
};

struct TwoDimSize{
	unsigned short Length;
	unsigned short Width;

	TwoDimSize(unsigned short newLen = 0, unsigned short newWidth = 0) :Length(newLen), Width(newWidth){}
	TwoDimSize& operator= (const TwoDimSize &Copy);
};

template<typename T> struct TwoDimSpace
{
	T **Table;
	TwoDimSize Tsize;

	TwoDimSpace(unsigned short Len, unsigned short Width, T val = NULL);
	TwoDimSpace();
	~TwoDimSpace();
};

template<typename T> struct KarnaughMap :TwoDimSpace<T>
{
	short **Index;
	KarnaughMap(unsigned short Len, unsigned short Width);
	KarnaughMap(char* FileName);
	~KarnaughMap();
};

template<typename T> T LogTwo(T Num);
short CntTrue(unsigned short Logic);
short CntTrue(const string &Logic, int NumOfVar = 4);
string ConvLogic(const unsigned short &Logic, int NumOfVar = 4);
int CanMerge(const string &, const string &);
bool Is_Cover(const string &, const string &);
void GetPrimeImplicant(Implicant* &, list <string>&);
bool SimplifyTable(list<string> &MinTerm, list<string> &PrimeImp, vector<string> &Pick);
string TranslateStr(const string &Logic);

int main()
{
	auto K = new KarnaughMap<char>("myMap.txt");
	int NumOfVar = LogTwo<int>(K->Tsize.Length) + LogTwo<int>(K->Tsize.Width);
	list<string> MinTerm;
	Implicant* MyImp = new Implicant[2];
	//Implicant S[4]{5, 5, 5};
	for (int i = 0; i < 2; ++i) {
		MyImp[i].Group = new myStruct[NumOfVar + 1];
	}
	for (int i = 0; i < K->Tsize.Width; ++i) {
		for (int j = 0; j < K->Tsize.Length; ++j) {
			switch (K->Table[j][i])
			{
			case '1':
				MinTerm.push_back(ConvLogic(K->Index[i][j]));
			case '2':
				MyImp[0].Group[CntTrue(K->Index[i][j])].push_back(pair<string, bool>(ConvLogic(K->Index[i][j]), true));
			case '0':
			default:
				break;
			}
		}
	}
	delete K;
	K = nullptr;
	/************************************************************************/
	{
		cout << "MinTerm :\n";
		list<string>::iterator Ans = MinTerm.begin(), EndA = MinTerm.end();
		cout << TranslateStr(*Ans++);
		while (Ans != EndA) {
			cout << " + " << TranslateStr(*Ans++);
		}
		cout << "\n";
	}
	/************************************************************************/
	list <string> PrimeImp;//PrimeImplicant
	{
		string TmpStr;
		bool Change = true;
		auto noRepeat = [&](const myStruct &a, const string &b)
		{
			for (auto&& i : a) {
				if (i.first == b) {
					return false;
				}
			}
			return true;
		};
		short tmpCnt(0);
		while (Change) {
			Change = false;
			for (int i = 0, Pos = -1; i < NumOfVar; ++i) {
				for (auto&& j : MyImp[0].Group[i]) {
					for (auto&& k : MyImp[0].Group[i + 1]) {
						if ((Pos = CanMerge(k.first, j.first)) >= 0) {
							k.second = false;
							j.second = false;
							TmpStr = k.first;
							TmpStr[Pos] = '-';
							tmpCnt = CntTrue(TmpStr);
							if (noRepeat(MyImp[1].Group[tmpCnt], TmpStr)) {
								MyImp[1].Group[tmpCnt].push_back(pair<string, bool>(TmpStr, true));
								Change = true;
							}
						}
					}
				}
			}
			for (int i = 0; i < 5; ++i) {
				for (auto&& j : MyImp[0].Group[i]) {
					if (j.second) {
						PrimeImp.push_back(move(j.first));
					}
				}
				MyImp[0].Group[i].clear();
			}
			swap(MyImp[0].Group, MyImp[1].Group);
		}
		//////////////////////////////////////////////////////////////////////////
		delete[] MyImp;
		vector<string> Pick;
		while (!SimplifyTable(MinTerm, PrimeImp, Pick)) {}

		vector<string>::iterator Ans = Pick.begin(), EndA = Pick.end();
		cout << "\nAns : " << TranslateStr(*Ans++);
		while (Ans != EndA) {
			cout << " + " << TranslateStr(*(Ans++));
		}
		cout << "\n";
				
	}
	return 0;
}

short CntTrue(unsigned short Logic)
{
	short Count = 0;
	do  {
		if (Logic & 1) {
			++Count;
		}
	} while (Logic >>= 1);
	return Count;
}

short CntTrue(const string &Logic, int NumOfVar /*= 4*/)
{
	short Count = 0;
	for (int i = 0; i < NumOfVar; ++i) {
		if (Logic[i] == '1') {
			++Count;
		}
	}
	return Count;
}

std::string ConvLogic(const unsigned short &Logic, int NumOfVar /*= 4*/)
{
	string Out(NumOfVar, '0');
	for (int i = NumOfVar - 1, Digit = 1; i >= 0; --i, Digit <<= 1)
	{
		if ((Logic & Digit) == Digit) {
			Out[i] = '1';
		}
	}

	return Out;
}

int CanMerge(const string &Logic1, const string &Logic2)
{
	int LSize = Logic1.size(), Count(0), Pos(0);
	for (int i = 0; i < LSize; ++i) {
		if (Logic1[i] != Logic2[i]) {
			++Count;
			Pos = i;
		}
	}
	return ((Count == 1) ? Pos : -1);
}

bool Is_Cover(const string &Logic1, const string &Logic2)
{
	int LSize = Logic1.size();
	for (int i = 0; i < LSize; ++i) {
		if ((Logic1[i] != Logic2[i]) && (Logic1[i] != '-')) {//??
			return false;
		}
	}
	return true;
}

template<typename T>
T LogTwo(T Num)
{
	T Cnt(0);
	while (Num >= 1) {
		Num >>= 1;
		++Cnt;
	}
	return Cnt;
}

bool SimplifyTable(list<string> &MinTerm, list<string> &PrimeImp, vector<string> &Pick)
{
	int PIsize(PrimeImp.size()), MTsize(MinTerm.size());
	auto Imp = new TwoDimSpace<short>(PIsize, MTsize, 0);//table[MTsize][PIsize]
	auto Cover = new vector<list<string>::iterator>[PIsize];
	auto PImpIndex = new list<string>::iterator[PIsize];
	vector<list<string>::iterator> IterCache[2];
	bool* Choose = new bool[PIsize], Change(false);
	TwoDimSize Table(0, 0);
	for (int i = 0; i < PIsize; ++i) {
		Choose[i] = true;
	}
	{
		int cnt(0);
		for (auto i = PrimeImp.begin(), endi = PrimeImp.end(); i != endi; ++i) {
			PImpIndex[cnt++] = i;
		}
	}
	auto noRepeat = [&](const vector<list<string>::iterator> &a, const list<string>::iterator &b)->bool
	{
		for (auto&& i : a) {
			if (*b == *i) {
				return false;
			}
		}
		return true;
	};
	auto Filter = [&](const int &Pos)->void
	{
		for (auto j = Cover[Pos].begin(), endj = Cover[Pos].end(); j != endj; ++j) {
			if (noRepeat(IterCache[0], *j)) {
				IterCache[0].push_back(*j);
			}
		}
		if (noRepeat(IterCache[1], PImpIndex[Pos])) {
			Pick.push_back(move(*PImpIndex[Pos]));
			IterCache[1].push_back(PImpIndex[Pos]);//need?
		}
	};
	auto STL_erase = [](list<string> &Item, vector<list<string>::iterator> &Unless)->void
	{
		for (auto&& Remove : Unless) {
			Item.erase(Remove);
		}
		Unless.clear();
	};
	for (auto i = MinTerm.begin(), endi = MinTerm.end(); i != endi; ++i, ++Table.Length) {
		Table.Width = 0;
		for (auto j = PrimeImp.begin(), endj = PrimeImp.end(); j != endj; ++j, ++Table.Width) {
			if (Is_Cover(*j, *i)) {
				++Imp->Table[Table.Length][Table.Width];
				Cover[Table.Width].push_back(i);
			}
			//cout << Imp->Table[Table.Length][Table.Width] << " ";
		}
		//cout << endl;
	}

	for (int i = 0, Cnt(0), Flag(0); i < Table.Length; ++i, Cnt = 0) {
		for (int j = 0; j < Table.Width; ++j) {
			if (Imp->Table[i][j] == 1) {
				Flag = j;
				++Cnt;
			}
		}
		if (Cnt == 1 && Choose[Flag]) {
			Filter(Flag);
			Choose[Flag] = false;
		}
	}
	if (IterCache[0].size() != MinTerm.size()) {
		auto Iter = PrimeImp.begin();
		int* CovSize = new int[PIsize];
		for (int i = 0; i < PIsize; ++i, ++Iter) {
			CovSize[i] = Cover[i].size();
			if (Choose[i] && CovSize[i] == 1) {
				IterCache[1].push_back(Iter);
				Choose[i] = false;
				Change = true;
			}
		}
		if (!Change) {
			int LargePos = 0;
			for (int i = 0; i < PIsize; ++i) {
				if ((CovSize[LargePos] < CovSize[i]) && Choose[i]) {
					LargePos = i;
				}
			}
			Filter(LargePos);
		}
		delete[] CovSize;
	}

	STL_erase(PrimeImp, IterCache[1]);
	STL_erase(MinTerm, IterCache[0]);
	delete Imp;
	delete[] Choose;
	delete[] Cover;
	delete[] PImpIndex;
	return (MinTerm.size() == 0);
}

string TranslateStr(const string &Logic)
{
	int Lsize = Logic.size();
	char *Out = new char[Lsize];
	int Count = 0;
	for (int i = 0; i < Lsize; ++i) {
		if (Logic[i] == '0') {
			Out[Count++] = 'a' + i;
		}
		else if (Logic[i] == '1') {
			Out[Count++] = 'A' + i;
		}
	}
	return (string(Out, Count));
}

template<typename T>
KarnaughMap<T>::KarnaughMap(unsigned short Len, unsigned short Width) :TwoDimSpace<T>(Len, Width)
{
	this->Table = new char *[Tsize.Width];
	this->Index = new short *[Tsize.Width];
	for (int i = 0; i < this->Tsize.Width; ++i) {
		this->Table[i] = new char[this->Tsize.Length];
	}
}

template<typename T>
KarnaughMap<T>::KarnaughMap(char* FileName)
{
	ifstream InFile(FileName);

	if (InFile.is_open()) {
		InFile.seekg(0, ios::beg);
		InFile >> this->Tsize.Length >> this->Tsize.Width;
		//cout << this->MapSize.Length << this->MapSize.Width << "\n";
		this->Table = new char *[Tsize.Width];
		this->Index = new short *[Tsize.Width];
		for (int i = 0; i < this->Tsize.Width; ++i) {
			this->Table[i] = new char[this->Tsize.Length];
			this->Index[i] = new short[Tsize.Length];
			for (int j = 0; j < this->Tsize.Length; ++j) {
				InFile >> this->Table[i][j];
				this->Index[i][j] = i * Tsize.Length + j;
				//cout << Table[i][j];
			}
			//cout << "\n";
			swap(this->Index[i][this->Tsize.Length - 1], this->Index[i][this->Tsize.Length - 2]);
		}
		swap(this->Index[this->Tsize.Width - 1], this->Index[this->Tsize.Width - 2]);
	}
	else {
		cerr << "\nFile could not be opened\n";
		exit(1);
	}
	/************************************************************************/
	/*for (int i = 0; i < this->Tsize.Length; ++i) {
	for (int j = 0; j < this->Tsize.Width; ++j) {
	cout << this->Index[j][i];
	}
	cout << endl;
	}*/
	/************************************************************************/
	InFile.close();
}

template<typename T>
KarnaughMap<T>::~KarnaughMap()
{
	for (int i = 0; i < this->Tsize.Width; ++i) {
		delete[] Index[i];
	}
	delete[] Index;
}

TwoDimSize& TwoDimSize::operator=(const TwoDimSize &Copy)
{
	this->Length = Copy.Length;
	this->Width = Copy.Width;
	return *this;
}

template<typename T>
TwoDimSpace<T>::TwoDimSpace() :Tsize(), Table(nullptr){}

template<typename T>
TwoDimSpace<T>::TwoDimSpace(unsigned short Len, unsigned short Width, T val) : Tsize(Len, Width)
{
	this->Table = new T *[Tsize.Width];
	for (int i = 0; i < this->Tsize.Width; ++i) {
		this->Table[i] = new T[this->Tsize.Length];
		for (int j = 0; j < Tsize.Length; ++j) {
			this->Table[i][j] = val;
		}
	}
}

template<typename T>
TwoDimSpace<T>::~TwoDimSpace()
{
	for (int i = 0; i < this->Tsize.Width; ++i) {
		delete[] this->Table[i];
	}
	delete[] this->Table;
}
