#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

#define ADDR_LEN 64
#define MIN(A,B) (A>B)?B:A

bool checkCommand(int argc, char** argv)
{
    if (argc != 9)
        return false;

    if (strcmp(argv[1],"nk") != 0)
        return false;

    if (strcmp(argv[3],"assoc") != 0)
        return false;

    if (strcmp(argv[5],"blocksize") != 0)
        return false;

    if (strcmp(argv[7],"repl") != 0)
        return false;

    return true;
}

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

int findReplace(string* strArr, int arrSize, string repl, int* freq)
{
    if (repl == "l")
    {
        int lessHit = INT_MIN;
        int lessHitIdx = 0;
        for (int i=0; i<arrSize; i++)
        {
            if (lessHit > freq[i])
            {
                lessHit = freq[i];
                lessHitIdx = i;
            }
        } 
        return lessHitIdx;
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
    if (!checkCommand(argc, argv))
    {
        cerr << "Invalid Command Line!" << endl;
        return -1;
    }

    int tagBits, setBits, blockBits;
    
    long nk, assoc, blockSize, numSets;
    if ((parseParameters(argv[2], &nk) \
            && parseParameters(argv[4], &assoc) \
            && parseParameters(argv[6], &blockSize)) != true)
    {
        return -1; 
    }
    if ((strcmp(argv[8], "l") != 0) && (strcmp(argv[8], "r") != 0))
    {
        cerr << "Invalid paremeter: " << argv[8] << endl;
        return -1;
    }
    string repl = argv[8];


    /* Construct a cache table */
    if (nk % assoc == 0) 
    {
        numSets = (int)nk/(int)assoc;
    }
    else
    {
        numSets = 0;
        cerr << "The Capacity of the cache must be divisible by the associative!" << endl;
        return -1;
    }
    string** cacheTable = new string* [numSets];
    for (int i=0; i<numSets; i++)
    {
        cacheTable[i] = new string [assoc];
    }

    /* Construct a table of the available index in certain set */
    int* indexOfSet = new int [numSets]();

    /* Construct a table to store the hit frequency */
    int** freqHit = new int* [numSets]();
    for (int i=0; i<numSets; i++)
    {
        freqHit[i] = new int [assoc];
    }

    /* Configure bits arrangement in address */
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

    /* For Debug
     * cout << "Tag bits: " << tagBits << endl;
     * cout << "Set bits: " << setBits << endl;
     * cout << "Block bits: " << blockBits << endl;
     */

    string cmd, oper, addr;
    string tag, block;
    long set;
    int addrSize; 
    char* buf;
    char* pErr;
    int readCount=0, writeCount=0;
    int readMiss=0, writeMiss=0;

    while (getline(cin, cmd) && !cmd.empty())
    {
        stringstream ss(cmd);
        ss >> oper >> addr;
        if (!ss || ((oper != "r") && (oper != "w")))
            cerr << "Invalid trace input!" << endl;

        cout << "oper: " << oper << " addr: " << addr << endl;

        // Padding '0' before the address    
        addrSize = addr.size();
        for (int i=0; i<ADDR_LEN-addrSize; i++)
            addr.insert(0,"0");

        buf = new char [tagBits];
        addr.copy(buf, tagBits, 0);
        buf[setBits] = '\0';
        // Construct string tag using char*
        tag = buf;
        delete [] buf;

        buf = new char [setBits];
        addr.copy(buf, setBits, tagBits);
        buf[setBits] = '\0';
        set = strtol(buf, &pErr, 16);
        if (*pErr || (set>numSets))
        {
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

        if (oper == "r")
        {
            readCount++;
            for (int i=0; i<assoc; i++)
            {
                if (cacheTable[set][i] != tag)
                    readMiss++;
                else
                    freqHit[set][i]++;
            }
        }
        else if (oper == "w")
        {
            writeCount++;
            for (int i=0; i<assoc; i++)
            {
                if (cacheTable[set][i] != tag)
                    writeMiss++;
                else
                    freqHit[set][i]++;
            }
        }
        if (indexOfSet[set] == assoc)
        {
            indexOfSet[set] = findReplace(cacheTable[set], assoc, repl, freqHit[set]);
        }
        cacheTable[set][indexOfSet[set]] = tag;
        indexOfSet[set] = findAvailableIndex(cacheTable[set], assoc);

    }

    cout << readMiss + writeMiss;
    cout << (readMiss + writeMiss)/(readCount + writeCount);
    cout << readMiss;
    cout << readMiss/readCount;
    cout << writeMiss;
    cout << writeMiss/writeCount << endl;
    
    /* Delete the cache table */
    for (int i=0; i<assoc; i++)
    {
        delete [] cacheTable[i];
    }
    delete [] cacheTable;

    delete [] indexOfSet;
    
    for (int i=0; i<assoc; i++)
    {
        delete [] freqHit[i];
    }
    delete [] freqHit;
    
    return 0;
}
