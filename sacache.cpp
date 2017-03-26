#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

const int WAY = 4;
class CacheLine
{
private:
	string word[8];
	int tag;
	int dirty;
	int count;

public:
	CacheLine()
		{
		 dirty = 0;
		 tag = 0;
		 count = 0;
		 for(int i = 0; i < 8; i++)
				word[i] = "00";
		}

	void setDirty(int dBit)
	{ dirty = dBit; }

	void setTag(int tTag)
	{ tag = tTag; }

	void setWord(int off, string data)
	{ word[7-off] = data; }

	int getDirty() const
	{ return dirty; }

	int getTag() const
	{ return tag; }

	string getWord(int off) const
	{ return word[7-off]; }

	int getCount() const
	{ return count; }

	void incCount()
	{ count = count + 1; }

	void resCount()
	{ count = 0; }
};

struct Ram
{
	string block[8];
};

int main(int argc, char *argv[])
{
	CacheLine cache[64];

	Ram *memory  = new Ram[8192];

	for(int i = 0; i < 8192; i++)
	{
		for(int a = 0; a < 8; a++)
			memory[i].block[a] = "00";
	}

	string tempAdd, op, data;
	int address, offset, setNum, tag, startingCacheLine, cacheLine, lru;
	int memLine, hitMiss, saveDirty, memAdd;

	bool stateTag;

	char *filename = argv[1];
	ifstream inFile(filename);
	
	ofstream outFile("sa-out.txt");

	/**/
	while (inFile >> tempAdd >> op >> data)
	{
		stringstream hexAdd;

		hexAdd << hex << tempAdd;
		hexAdd >> address;

		offset = address & 0x7;

		setNum = (address >> 3) & 0xf;

		startingCacheLine = setNum * WAY;

		tag = (address >> 7) & 0x1ff;


		if(op == "FF") //write function
		{
			for(int i = 0; i < WAY; i++)
			{
				if(cache[startingCacheLine+i].getTag() != tag)
					stateTag = false;
				else
				{
					stateTag = true;
					cacheLine = startingCacheLine+i;
					i = WAY;
				}
			}

			if(!stateTag)
			{
				lru = startingCacheLine;

				for(int i = 0; i < WAY; i++)
				{
					if(cache[startingCacheLine+i].getCount() > cache[lru].getCount())
						lru = startingCacheLine+i;
				}

				memAdd = tag;
				memAdd = (memAdd << 4) | setNum;

				memLine = cache[lru].getTag();
				memLine = (memLine << 4) | setNum;

				if(cache[lru].getDirty())
				{
					for(int i = 0; i < 8; i++)
						memory[memLine].block[i] = cache[lru].getWord(i);
				}
				for(int i = 0; i < 8; i++)
					cache[lru].setWord(i, memory[memAdd].block[i]);
				
				cache[lru].setTag(tag);

				for(int i = 0; i < WAY; i++)
					cache[startingCacheLine+i].incCount();

				cache[lru].resCount();
				cache[lru].setWord(offset, data);
				cache[lru].setDirty(1);
			}
			else
			{
				cache[cacheLine].setWord(offset, data);
				cache[cacheLine].setDirty(1);

				for(int i = 0; i < WAY; i++)
					cache[startingCacheLine+i].incCount();

				cache[cacheLine].resCount();

			}

		}

		else //read function
		{
			for(int i = 0; i < WAY; i++)
			{
				if(cache[startingCacheLine+i].getTag() != tag)
					stateTag = false;
				else
				{
					stateTag = true;
					cacheLine = startingCacheLine+i;
					i = WAY;
				}
			}

			if(stateTag)
			{
				hitMiss = 1;
				saveDirty = cache[cacheLine].getDirty();
			
				for(int i = 0; i < WAY; i++)
					cache[startingCacheLine+i].incCount();


				cache[cacheLine].resCount();

				outFile << tempAdd << " ";

				for(int i = 7; i >= 0; i--)
					outFile << cache[cacheLine].getWord(i);
			
				outFile << " " << hitMiss << " " << saveDirty << endl;
			}
			else
			{
				hitMiss = 0;

				lru = startingCacheLine;

				for(int i = 0; i < WAY; i++)
				{
					if(cache[startingCacheLine+i].getCount() > cache[lru].getCount())
						lru = startingCacheLine+i;
				}

				memLine = cache[lru].getTag();
				memLine = (memLine << 4) | setNum;

				memAdd = tag;
				memAdd = (memAdd << 4) | setNum;

				if(cache[lru].getDirty())
				{
					for(int i = 0; i < 8; i++)
						memory[memLine].block[i] = cache[lru].getWord(i);
				}
				
				for(int i = 0; i < 8; i++)
					cache[lru].setWord(i, memory[memAdd].block[i]);

				saveDirty = cache[lru].getDirty();

				cache[lru].setDirty(0);
				cache[lru].setTag(tag);

				for(int i = 0; i < WAY; i++)
					cache[startingCacheLine+i].incCount();

				cache[lru].resCount();

				outFile << tempAdd << " ";

				for(int i = 7; i >= 0; i--)
					outFile << cache[lru].getWord(i);
			
				outFile << " " << hitMiss << " " << saveDirty << endl;
			}
		}
		

	}
	inFile.close();
	outFile.close();
	//system("pause>nul");

}