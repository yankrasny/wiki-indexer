/*
 * diff.cc
 *
 *  Created on: May 15, 2011
 *      Author: jhe
 */

#include        <cstdio>
#include        <cstdlib>
#include        <cstring>
#include        <map>
#include	<iterator>
#include	<fstream>
#include	<vector>
#include	<ctime>


#define MAXSIZE 250939073

struct binfo
{
    int total, total_dis, post;
};

#include "general_partition.h"


//#define PRINTTREE 1
int* buf;
int* wcounts;
int* wsizes;
partitions<CutDocument2>* par;

int ndoc = 0;
int* hbuf;
int* mhbuf;
unsigned char* tree_buf;

//double wsizes2[20] = {0.01,0.05, 0.07, 0.09,  0.1, 0.5, 1, 3, 5, 7, 9, 10, 15, 20, 25, 30, 35, 40, 45, 50};

int dothejob(int* buf, int* wcounts, int id, int vs, const vector<double>& wsizes2)
{
    char fn[256];
    memset(fn, 0, 256);
    sprintf(fn, "/data4/jhe/proximity/model/test/%d", id);
    FILE* f = fopen(fn, "r");
    int total = 1;
    int a,b,c,d,e,fs;
    while ( fscanf(f, "%d\t%d\t%d\t%d\t%d\t%d\n", &a, &b, &c, &d, &e, &fs)>0){
        wsizes[total] = a;
        total++;
    }
    wsizes[0] = 5;
    fclose(f);
    memset(fn, 0, 256);
    sprintf(fn, "/data4/jhe/proximity/general/meta/%d", id);
    f = fopen(fn, "rb");
    int size;
    fread(&size, sizeof(unsigned), 1, f);
    fread(mhbuf, sizeof(unsigned), size, f);
    fclose(f);
    memset(fn, 0, 256);
    sprintf(fn, "/data4/jhe/proximity/minWinnowing/meta/%d", id);
    f = fopen(fn, "rb");
    fread(&size, sizeof(unsigned), 1, f);
    fread(hbuf, sizeof(unsigned), size, f);
    fclose(f);


    iv* trees = new iv[vs];
    int wptr = 0;
    int hptr = 0;
    par->init();

    for ( int i = 0; i < vs; i++){
        if ( wcounts[i] > 0 )
            par->fragment(i, &buf[wptr], wcounts[i], &mhbuf[hptr], &hbuf[hptr],  &trees[i], wsizes, total);
        wptr += wcounts[i];
        if (wcounts[i] > B)
        {
            hptr += (wcounts[i]-B+1);
        }

        trees[i].complete();
    }

    printf("finish sorting!\n");
    binfo binf;
    memset(fn, 0, 256);
    sprintf(fn, "test/%d.2", id);
    f = fopen(fn, "w");
    int length = wsizes2.size();
    for ( int i = 0; i < length; i++)
    {
        double wsize = wsizes2[i];
        par->completeCount(wsize);
        par->select(trees, wsize);
        par->PushBlockInfo(buf, wcounts,id, vs, &binf);
        fprintf(f, "%.2lf\t%d\t%d\t%d\n", wsize, binf.total_dis, binf.total, binf.post);
    }
    fclose(f);
    par->dump_frag(id);
    delete[] trees;
    ndoc += vs;
    return 0;
}

int main(int argc, char**argv)
{
    if ( argc < 2)
    {
        printf("diff:para startIdx endIdx basicLayer\n");
        exit(0);
    }
    par = new partitions<CutDocument2>(atoi(argv[1]), atoi(argv[4]), atoi(argv[5]));
    wsizes = new int[1000];
    buf = new int[MAXSIZE];
    mhbuf = new int[MAXSIZE];
    hbuf = new int[MAXSIZE];
    tree_buf = new unsigned char[MAXSIZE];
    FILE* fcomplete = fopen("/data/jhe/wiki_access/completeFile", "rb");
    FILE* fnumv = fopen("/data/jhe/wiki_access/numv", "rb");
    int doccount;
    fread(&doccount, sizeof(unsigned), 1, fnumv);
    int* numv = new int[doccount];
    fread(numv, sizeof(unsigned), doccount, fnumv);

    FILE* fword_size = fopen("/data/jhe/wiki_access/word_size", "rb");
    unsigned total_ver;
    fread(&total_ver, sizeof(unsigned), 1, fword_size);
    wcounts = new int[total_ver];
    fread(wcounts, sizeof(unsigned), total_ver, fword_size);
    fclose(fword_size);

    int ptr = 0;

    ifstream fin("options");
    istream_iterator<double> data_begin(fin);
    istream_iterator<double> data_end;
    vector<double> wsizes2(data_begin, data_end);
    fin.close();

    for ( int i = 0; i < doccount; i++)
    {
        int total = 0;
        for ( int j = 0; j < numv[i]; j++)
            total+=wcounts[ptr+j];

        fread(buf, sizeof(unsigned), total, fcomplete);
        if ( i < atoi(argv[2]))
        {
            ptr += numv[i];
            continue;
        }
        dothejob(buf, &wcounts[ptr], i, numv[i], wsizes2);
        printf("Complete:%d\n", i);
        ptr += numv[i];
        if( i >= atoi(argv[3]))
            break;
    }

    FILE* fscud = fopen("SUCCESS!", "w");
    fclose(fscud);

    delete par;
    return 0;
}
