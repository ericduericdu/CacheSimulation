#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class CacheLine
{
private:
	string word[8];
	int tag;
	int dirty;

public:
	CacheLine()
		{
		 dirty = 0;
		 tag = 0;
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


};

struct Ram
{
	string block[8];
};

int main(int argc, char *argv[])
{
	CacheLine cache[64];

	Ram *memory  = new Ram[8192];

	for(int i = 0; i < 8129; i++)
	{
		memory[i].block[0] = "00";
		memory[i].block[1] = "00";
		memory[i].block[2] = "00";
		memory[i].block[3] = "00";
		memory[i].block[4] = "00";
		memory[i].block[5] = "00";
		memory[i].block[6] = "00";
		memory[i].block[7] = "00";

	}

	string tempAdd, op, data;
	int address, offset, cacheLine, tag; 
	int memLine, hitMiss, saveDirty, memAdd;

	char *filename = argv[1];
	ifstream inFile(filename);

	ofstream outFile("dm-out.txt");

	while (inFile >> tempAdd >> op >> data)
	{
		stringstream hexAdd;

		hexAdd << hex << tempAdd;
		hexAdd >> address;

		offset = address & 0x7;

		cacheLine = (address >> 3) & 0x3f;

		tag = (address >> 9) & 0x7f;

		
		if(op == "FF") //write function
		{	
			if(cache[cacheLine].getTag() != tag)
			{
				memLine = cache[cacheLine].getTag();
				memLine = (memLine << 6) | cacheLine;

				
				memAdd = tag;
				memAdd = (memAdd << 6) | cacheLine;


				if(cache[cacheLine].getDirty())
				{
					for(int i = 0; i < 8; i++)
						memory[memLine].block[i] = cache[cacheLine].getWord(i);
				}

				for(int i = 0; i < 8; i++)
					cache[cacheLine].setWord(i, memory[memAdd].block[i]);

				cache[cacheLine].setTag(tag);
			}
			cache[cacheLine].setDirty(1);
			cache[cacheLine].setWord(offset, data);
			
		}
		else //read function
		{
			saveDirty = cache[cacheLine].getDirty();

			if(cache[cacheLine].getTag() == tag)
				hitMiss = 1;
			else
			{
				hitMiss = 0;

				memLine = cache[cacheLine].getTag();
				memLine = (memLine << 6) | cacheLine;
				
				memAdd = tag;
				memAdd = (memAdd << 6) | cacheLine;

				if(cache[cacheLine].getDirty())
				{
					for(int i = 0; i < 8; i++)
						memory[memLine].block[i] = cache[cacheLine].getWord(i);
				}
				for(int i = 0; i < 8; i++)
					cache[cacheLine].setWord(i, memory[memAdd].block[i]);

				cache[cacheLine].setTag(tag);
				cache[cacheLine].setDirty(0);
			}

			outFile << tempAdd << " ";

			for(int i = 7; i >= 0; i--)
				outFile << cache[cacheLine].getWord(i);
			
			outFile << " " << hitMiss << " " << saveDirty << endl;
		}
	}

	 inFile.close();
	 outFile.close();

	//system("pause>nul");
	return 0;
}
