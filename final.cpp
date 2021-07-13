#include <iostream>
#include  <iomanip>
#include  <cmath>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(48)    /////////////////////////////////ret

const int N_WAY = 3; /////////////////////////////////ret
const int CACHE_LINE_SIZE = 2;/////////////////////////////////////////ret
const double CACHE_SETS = ceil(CACHE_SIZE/(N_WAY* CACHE_LINE_SIZE));
enum cacheResType { MISS = 0, HIT = 1 };

/* The following implements a random number generator */
unsigned int m_w = 0xABCCAB99;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0xDEAD6902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16); // 65535 is CACHE_SIZE-1
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGenA()
{
	static unsigned int addr = 0;
	return (addr++) % (DRAM_SIZE);
}

unsigned int memGenB()
{
	static unsigned int addr = 0;
	return  rand_() % (64 * 1024);
}

unsigned int memGenC()
{
	static unsigned int a1 = 0, a0 = 0;
	a0++;
	if (a0 == 512) { a1++; a0 = 0; }
	if (a1 == 128) a1 = 0;
	return(a1 + a0 * 128);
}

unsigned int memGenD()
{
	return  rand_() % (16 * 1024);
}

unsigned int memGenE()
{
	static unsigned int addr = 0;
	return (addr++) % (1024 * 64);
}

unsigned int memGenF()
{
	static unsigned int addr = 0;
	return (addr += 64) % (64 * 4 * 1024);
}


// Direct Mapped Cache Simulator
//unsigned int** CACHE = new unsigned int* [CACHE_SETS];
//
//for (int i = 0; i < CACHE_SETS; i++)
//{
//	CACHE[i] = new unsigned int[N_WAY];
//}

void RandRep(unsigned int** &CACHE,  int index, unsigned int tag, unsigned int tag_bits)
{
	srand(time(NULL));
	unsigned int r = rand() % N_WAY;
	CACHE[index][r] = (1 << (tag_bits + (CACHE_LINE_SIZE * 8))) | (tag << (CACHE_LINE_SIZE * 8));
	//cout << r << CACHE[index][r];
	return;
}
void addCache(unsigned int** &CACHE,  int index, unsigned int tag, unsigned int tag_bits, unsigned int address) /// still did not examine rep case
{
	for (int j = 0; j < N_WAY; j++)
	{
		unsigned int current_cache_line = CACHE[index][j];
		unsigned int valid = (current_cache_line >> (CACHE_LINE_SIZE*8 + tag_bits));
		if (valid!= 1)
		{
			CACHE[index][j] = (1 << (tag_bits + (CACHE_LINE_SIZE*8))) | (tag << (CACHE_LINE_SIZE*8));
			//valid = 1;
			return;
		}

		
	}


}
cacheResType cacheSimDM(unsigned int addr, unsigned int** &CACHE, int* &indices_count)
{
	bool old_entry = false;
	int set_index;
	unsigned int tag, offset, indexc, tagc, current_cache_line, addrtemp;
	offset = addr % CACHE_LINE_SIZE;
	addrtemp = addr >> int(log2(CACHE_LINE_SIZE));
	set_index = addrtemp % int(CACHE_SETS);
	addrtemp = addrtemp >> int(log2(CACHE_SETS));
	tag = addrtemp;
	int tag_bits = 8 - int(log2(CACHE_LINE_SIZE)) - int(log2(CACHE_SETS)); ///////////////////////////////////////////////ret
	for (int j = 0; j < N_WAY; j++)
	{
		current_cache_line = CACHE[set_index][j];
		tagc = (current_cache_line / int(pow(2, CACHE_LINE_SIZE * 8))) % int(pow(2, tag_bits));
		//tagc = (current_cache_line >> (CACHE_LINE_SIZE * 8)) & (0xf);
		if (tag == tagc)
		{
			old_entry = true;
			j = N_WAY + 1;
		}
	}

	if (old_entry==false)
	{
		*(indices_count + set_index) += 1;
	}
	
	for (int j = 0; j < N_WAY; j++)
	{
		current_cache_line = CACHE[set_index][j];
		//int tag_bits = 26 - int(log2(CACHE_LINE_SIZE)) - int(log2(CACHE_SETS));
		tagc = (current_cache_line / int(pow(2, CACHE_LINE_SIZE * 8))) % int(pow(2, tag_bits));
		//tagc = (current_cache_line >> (CACHE_LINE_SIZE * 8)) & (0xf); ////////////change
		if ((tagc == tag))
		{
			return HIT;
		}
		
	}
	if (*(indices_count + set_index) > N_WAY)
	{
		RandRep(CACHE, set_index, tag, tag_bits);
		return MISS;
	}
	// This function accepts the memory address for the memory transaction and 
	// returns whether it caused a cache miss or a cache hit	// The current implementation assumes there is no cache; so, every transaction is a miss
	addCache(CACHE, set_index, tag, tag_bits, addr);
	return MISS;
}


string msg[2] = { "Miss", "Hit"};

#define		NO_OF_Iterations	14		// CHange to 1,000,000
int main()
{
	int* indices_count = new int [CACHE_SETS];
	for (int i = 0; i < CACHE_SETS; i++)
	{
		indices_count[i] = 0;
	}
	unsigned int** CACHE = new unsigned int* [CACHE_SETS];

	for (int i = 0; i < CACHE_SETS; i++)
	{
		CACHE[i] = new unsigned int[N_WAY];
	}

	//for (int i = 0; i < CACHE_SETS; i++)
	//{
	//	for (int j = 0; j < N_WAY; j++)
	//	{
	//		CACHE[i][j] = 0;   ////assume address zero is to get stored ..... revise
	//	}
	//}
	unsigned int hit = 0;
	cacheResType r;

	unsigned int addr;
	cout << "Cache Simulator\n";

	unsigned int mem [14] = {3, 180, 43, 2, 190, 88, 191, 14, 31, 181, 191, 186, 46, 206};
	for (int inst = 0; inst < NO_OF_Iterations; inst++)
	{
		//addr = memGenB();
		addr = mem[inst];
		r = cacheSimDM(addr, CACHE, indices_count);
		if (r == HIT) hit++;
		cout << "0x" << setfill('0') << setw(8) << hex << addr << " (" << msg[r] << ")\n";
	}
	//cout << "Hit ratio = " << (100 * hit / NO_OF_Iterations) << endl;

	//for (int i = 0; i < CACHE_SETS; i++)
	//{
	//	//cout << *(indices_count + i) << "\n";
	//	for (int j = 0; j < N_WAY; j++)
	//	{
	//		cout << CACHE[i][j] << "\n";
	//	}
	//}

	system("pause");
	return 0;
}
//{"mode":"full", "isActive" : false}