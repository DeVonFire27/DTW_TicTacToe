#pragma once
#include "SLList.h"

template<typename Type> class HTable
{

private:
	SLList<Type>* Buckets;
	unsigned int BuckSize;
	unsigned int (*FuncPtr) (const Type &);

public:
	HTable(unsigned int numOfBuckets, unsigned int (*hFunction) (const Type &v));
	~HTable();
	HTable<Type>& operator=(const HTable<Type>& that);
	HTable(const HTable<Type>& that);
	void insert(const Type& v);
	bool findAndRemove(const Type& v);
	void clear();
	int find(const Type& v) const;
	void printSomeStuff(const char* filePath = "hashdata.txt");
	SLList<Type>& operator[] (const unsigned int index);

};

/////////////////////////////////////////////////////////////////////////////
// Function : Constructor 
// Parameters : numOfBuckets - the number of buckets
//              hFunction - pointer to the hash function for this table
// Notes : constructs an empty hash table
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
HTable<Type>::HTable(unsigned int numOfBuckets, unsigned int (*hFunction) (const Type &v))
{
	Buckets = new SLList<Type>[numOfBuckets];
	BuckSize = numOfBuckets;
	FuncPtr = hFunction;
}

/////////////////////////////////////////////////////////////////////////////
// Function : Destructor
// Notes : destroys a hash table
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
HTable<Type>::~HTable()
{
	delete [] Buckets;
}


/////////////////////////////////////////////////////////////////////////////
// Function : Assignment Operator
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
HTable<Type>& HTable<Type>::operator=(const HTable<Type>& that)
{
	delete [] Buckets;
	Buckets = new SLList<Type>[that.BuckSize];
	for(decltype(that.BuckSize) x = 0; x < that.BuckSize; x++)
		Buckets[x] = that.Buckets[x];
	BuckSize = that.BuckSize;
	FuncPtr = that.FuncPtr;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Function : Copy Constructor
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
HTable<Type>::HTable(const HTable<Type>& that)
{
	Buckets = new SLList<Type>[that.BuckSize];
	for(int x = 0; x < that.BuckSize; x++)
		Buckets[x] = that.Buckets[x];
	BuckSize = that.BuckSize;
	FuncPtr = that.FuncPtr;
}

/////////////////////////////////////////////////////////////////////////////
// Function : insert
// Parameters : v - the item to insert into the hash table
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
void HTable<Type>::insert(const Type& v)
{
	unsigned int index = FuncPtr(v);
	Buckets[index].addHead(v);
}

/////////////////////////////////////////////////////////////////////////////
// Function : findAndRemove
// Parameters : v - the item to remove(if it is found)
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
bool HTable<Type>::findAndRemove(const Type& v)
{
	unsigned int index = FuncPtr(v);
	SLLIter<Type> i(Buckets[index]);
	for(i.begin(); !i.end(); ++i)
	{
		if(i.current() == v)
		{
			Buckets[index].remove(i);
			return true;
		}
	}
	return false;

}

/////////////////////////////////////////////////////////////////////////////
// Function : clear
// Notes : clears the hash table
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
void HTable<Type>::clear()
{
	for(decltype(BuckSize) x = 0; x < BuckSize; x++)
		Buckets[x].clear();
}

/////////////////////////////////////////////////////////////////////////////
// Function : find
// Parameters : v - the item to look for
// Return : the bucket we found the item in or -1 if we didn’t find the item.
/////////////////////////////////////////////////////////////////////////////
template<typename Type>
int HTable<Type>::find(const Type& v) const
{
	unsigned int index = FuncPtr(v);
	SLLIter<Type> i(Buckets[index]);
	for(i.begin(); !i.end(); ++i)
	{
		if(i.current() == v)
			return index;
	}
	return -1;
}
// the following variable names are used below, match them to your variable names 
//	theTable - this is the array of lists
//	buck - the number of buckets

template <typename Type>
void HTable<Type>::printSomeStuff(const char* filePath)
{
	ofstream outFile(filePath);

	if(outFile.is_open())
	{
		unsigned int empty = 0;
		unsigned int totalCount = 0;
		unsigned int loIndex = 0;
		unsigned int hiIndex = 0;
		
		for(unsigned int i = 0; i < BuckSize; ++i)
		{
			totalCount += Buckets[i].size();
			outFile << i << " : " << Buckets[i].size() << '\n';
			if(Buckets[i].size() == 0)
				++empty;
				
			if(Buckets[i].size() < Buckets[loIndex].size())
				loIndex = i;
			else if(Buckets[i].size() > Buckets[hiIndex].size())
				hiIndex = i;
		}

		outFile << '\n' << totalCount << " Total items stored in " << BuckSize << " buckets\n";
		outFile << '\n' <<empty << " Buckets are empty\n\n";

		unsigned int Low = Buckets[loIndex].size();
		unsigned int range = Buckets[hiIndex].size() - Low + 1;

		outFile << "each bucket contains between " << Low << " and " << Low+range-1 << " items.\n\n";

		unsigned int* arr = new unsigned int[range];
		for(unsigned int j = 0; j < range; ++j)
			arr[j] = 0;

		for(unsigned int k = 0; k < BuckSize; ++k)
			++arr[Buckets[k].size() - Low];

		for(unsigned int p = 0; p < range; ++p)
			outFile << arr[p] << " buckets have " << p+Low << " items\n";

		delete[] arr;
	}
}

template <typename Type>
SLList<Type>& HTable<Type>::operator[] (const unsigned int index)
{
	return Buckets[index];
}

