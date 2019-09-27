#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

#define ADDR_LEN 64
#define MIN(A,B) (A>B)?B:A

int parseParameters(char* argStr, long* pPar)
{
    char* pErr;
    *pPar = strtol(argStr, &pErr, 10);
    if (*pErr || (*pPar>INT_MAX))
    {
        cerr << "Invalid parameter: " << argStr << endl;
        return false;
    }
    return true;
}

/* Input: positive num
 * Output: log2(num), if log2(num) is integer 
 *         -1       , else
 */
int myLog2(int num)
{
    int pow=0;
    while ((num%2)==0)
    {
        num /= 2;
        pow++;
    }
    if (num == 1)
       return pow;
    else
       return -1; 
}

int findReplace(int* timeArr, int arrSize, string repl)
{
    if (repl == "l")
    {
        int cycle = INT_MAX;
        int lru = 0;
        for (int i=0; i<arrSize; i++)
        {
            if (cycle > timeArr[i])
            {
                cycle = timeArr[i];
                lru = i;
            }
        } 
        return lru;
    }
    else if (repl == "r")
    {
        srand(time(0));
        return (rand()%arrSize);
    }
    return 0;
}

int findAvailableIndex(string* strArr, int arrSize)
{
    for (int i=0; i<arrSize; i++)
    {
        if (strArr[i] == "")
            return i;
    }
    return arrSize;
}

int main(int argc, char** argv)
{
    /* Check and Parse parameters */
    if (argc != 5)
    {
        cerr << "Invalid number of parameters!" << endl;
        return -1;
    }

    int tagBits, setBits, blockBits;
    long nk, assoc, blockSize, numSets;
    
    if ((parseParameters(argv[1], &nk) \
            && parseParameters(argv[2], &assoc) \
            && parseParameters(argv[3], &blockSize)) != true)
    {
        return -1; 
    }
    if ((strcmp(argv[4], "l") != 0) && (strcmp(argv[4], "r") != 0))
    {
        cerr << "Invalid paremeter: " << argv[4] << endl;
        return -1;
    }
    string repl = argv[4];

    /* Calculate the number of sets */
    if (nk % assoc == 0) 
    {
        numSets = (int)nk*1024/(int)assoc;
    }
    else
    {
        numSets = 0;
        cerr << "The Capacity of the cache must be divisible by the associative!" << endl;
        return -1;
    }

    /* Construct a cache table */
    string** cacheTable = new string* [numSets];
    for (int i=0; i<numSets; i++)
    {
        cacheTable[i] = new string [assoc];
    }
    /* Construct a table of the available index in certain set */
    int* indexOfSet = new int [numSets]();
    /* Construct a table to store the access time of each block */
    int** timeAccess = new int* [numSets]();
    for (int i=0; i<numSets; i++)
    {
        timeAccess[i] = new int [assoc];
    }

    /* Configure number of bits of tag, set, block in the address */
    if ((blockBits = myLog2(blockSize)) == -1)
    {
        cerr << "The block size is not power of 2!" << endl;
        return -1;
    }
    if ((setBits = myLog2(numSets)) == -1)
    {
        cerr << "The # of sets is not power of 2!" << endl;
        return -1;
    }
    tagBits = ADDR_LEN - blockBits - setBits;

    /* For debug */
    // cout << "Tag bits: " << tagBits << endl;
    // cout << "Set bits: " << setBits << endl;
    // cout << "Block bits: " << blockBits << endl;
    

    string cmd, oper, hexAddr, addr;
    string tag, block;
    long set;
    int addrBits; 
    char *buf, *pErr;
    stringstream ssDec;
    unsigned long tmpDec;

    int hitCount=0;
    int readCount=0, writeCount=0;
    int readMiss=0, writeMiss=0;
    int cycle=0;
    bool hit=false;

    while (getline(cin, cmd) && !cmd.empty())
    {
        cycle++;
        stringstream ssCmd(cmd);
        ssCmd >> oper >> hexAddr;
        if (!ssCmd || ((oper != "r") && (oper != "w")))
            cerr << "Invalid trace input!" << endl;
        // if (hexAddr == "0")
        // {
        //     if (oper == "r")
        //     {
        //         readCount++;
        //         readMiss++;
        //     }
        //     else if (oper == "w")
        //     {
        //         writeCount++;
        //         writeMiss++;
        //     }
        //     continue;
        // }

        stringstream ssDec(hexAddr);
        ssDec >> hex >> tmpDec;
        addr = bitset<64>(tmpDec).to_string();

        /* For debug */
        // cout << "binary addr: " << addr << endl;

        buf = new char [tagBits];
        addr.copy(buf, tagBits, 0);
        buf[tagBits] = '\0';
        // Construct string tag using char*
        tag = buf;
        delete [] buf;

        buf = new char [setBits];
        addr.copy(buf, setBits, tagBits);
        buf[setBits] = '\0';
        set = strtol(buf, &pErr, 2);
        if (*pErr || (set>numSets))
        {
            *pErr = 0;
            cerr << "Invalid address: set number (" << set << ") larger than " 
                << "number of sets (" << numSets << ")" << endl;
            continue;
        }
        delete [] buf;

        buf = new char [blockBits];
        addr.copy(buf, blockBits, tagBits+setBits);
        buf[setBits] = '\0';
        // Construct string tag using char*
        block = buf;
        delete [] buf;

        /* For debug */
        // cout << "tag: " << tag << endl;
        // cout << "set: " << set << endl;

        for (int i=0; i<assoc; i++)
        {
            if (cacheTable[set][i] == tag)
            {
                hit = true;
                hitCount++;
                /* Record the last cycle in which the block access */
                timeAccess[set][i] = cycle;
                /* For debug */
                // cout << "[HIT] set, tag = " << set << " " << tag << endl;
                // cout << "Hit Counts: " << hitCount << endl;
                break;
            }
        }

        if (oper == "r")
            readCount++;
        else if (oper == "w")
            writeCount++;

        /* A miss occur */
        if (!hit)
        {
            if (oper == "r")
            {
                readMiss++;
            }
            else if (oper == "w")
            {
                writeMiss++;
            }
            
            /* Determine which block to store either the read data accessed 
             * from the memory or the write data given by the processor. 
             * If the all the block in the set are occupied (indexOfSet will 
             * return the number of blocks), find another block to replace
             * using the specified method, LRU or random. */
            if (indexOfSet[set] == assoc)
            {
                indexOfSet[set] = findReplace(timeAccess[set], assoc, repl);
                cout << "set[" << set << "]: replace index " << indexOfSet[set]
                    << ", cycle " << timeAccess[set][indexOfSet[set]] << endl;
            }
            /* If miss, put tag on the block regardless of read/write */
            cacheTable[set][indexOfSet[set]] = tag;
            /* Record the last cycle in which the block access */
            timeAccess[set][indexOfSet[set]] = cycle;
            /* Find available block for the next miss */
            indexOfSet[set] = findAvailableIndex(cacheTable[set], assoc);
        }
        hit = false;
    }

    cout << "Total Counts: " << readCount+writeCount << endl;
    cout << "Hit Counts: " << hitCount << endl;
    cout << readMiss + writeMiss << " ";
    cout << (double)(readMiss + writeMiss)/(readCount + writeCount)*100 << "% ";
    cout << readMiss << " ";
    cout << (double)readMiss/readCount*100 << "% ";
    cout << writeMiss << " ";
    cout << (double)writeMiss/writeCount*100 << "%" << endl;
    
    /* Delete the cache table */
    for (int i=0; i<assoc; i++)
    {
        delete [] cacheTable[i];
    }
    delete [] cacheTable;

    delete [] indexOfSet;
    
    for (int i=0; i<assoc; i++)
    {
        delete [] timeAccess[i];
    }
    delete [] timeAccess;
    
    return 0;
}
