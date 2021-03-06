#ifndef IV_H_
#define IV_H_

#include <stdio.h>
#include <stdlib.h>
#include "vb.h"
#include "readwritebits.h"
#include <list>
#include <map>

using namespace std;
struct frag_ptr
{
    int ptr;
    int off;
};

bool operator == (const frag_ptr& f1, const frag_ptr& f2)
{
    if ( f1.ptr == f2.ptr && f1.off == f2.off)
        return true;
    else
        return false;
}

struct node
{
    int nid;
    frag_ptr fptr;
    int start;
    int end;
};

int compareiv(const void* a1, const void* a2)
{
    node* n1 = (node*)a1;
    node* n2 = (node*)a2;

    int ret = n1->start - n2->start;
    int ret2 = n1->end - n2->end;
    if ( ret == 0)
        return ret2;
    else
        return ret;
}

class iv
{
public:
    iv()
    {
        nID = 0;
        nbuf = new node[1000000];
        sort_nbuf = new node[1000000];
    }

    ~iv()
    {
        delete[] nbuf;
        delete[] sort_nbuf;
        delete[] inlist;
    }

    int nID;
    list<int>* inlist;
    node* nbuf;
    node* sort_nbuf;

    int storeiv(unsigned char* buf)
    {
        int id = nID;
        unsigned char* buf_ptr = buf;
        VBYTE_ENCODE(buf_ptr, id);

        for ( int i = 0; i < id; i++)
        {
            int size = inlist[i].size();
            VBYTE_ENCODE(buf_ptr, size);
            for ( list<int>::iterator its = inlist[i].begin(); its != inlist[i].end(); its++)
            {
                int k = (*its);
                VBYTE_ENCODE(buf_ptr, k);
            }
        }

        return buf_ptr - buf;
    }

    int loadiv(unsigned char* buf)
    {
        unsigned char* buf_ptr =buf;
        int id;
        VBYTE_DECODE(buf_ptr, id);
        nID = id;
        inlist = new list<int>[id];
        nbuf = new node[id];
        for ( int i = 0; i < id;i++){
            int size;
            VBYTE_DECODE(buf_ptr, size);
            for ( int j = 0; j < size; j++)
            {
                int k;
                VBYTE_DECODE(buf_ptr, k);
                inlist[i].push_back(k);
            }
        }
    }
/* 
 * insert a fragment information to the inverted list structure
 */
    int insert(const frag_ptr& finfo, int start, int end)
    {
        end--;
        nbuf[nID].fptr = finfo;
        nbuf[nID].start = start;
        nbuf[nID].end = end;
        nbuf[nID].nid = nID;
        sort_nbuf[nID] = nbuf[nID];
        nID++;
        if ( nID > 1000000)
        {
            printf("memory out !\n");
            exit(0);
        }
        return nbuf[nID-1].nid;
    }

    bool intersect(const node& n1, const node& n2)
    {
        if ( n1.start > n2.end || n1.end < n2.start)
            return false;
        else
            return true;
    }

    int complete()
    {
        inlist = new list<int>[nID];
        for ( int i = 0; i < nID; i++)
        {
            for ( int j = i+1; j < nID; j++)
            {
                if ( intersect(sort_nbuf[i], sort_nbuf[j])){
                    int idx = sort_nbuf[i].nid;
                    int idx2 = sort_nbuf[j].nid;
                    inlist[idx].push_back(idx2);
                    inlist[idx2].push_back(idx);
                }
                //else
                //  break;
            }
        }
    }

    int find_conflict(int nID, vector<p>& li, int idx, frag_ptr* fbuf)
    {
        int size = inlist[nID].size();
        list<int>::iterator its;
        int ptr = 0;
        for ( its = inlist[nID].begin(); its!=inlist[nID].end(); its++)
        {
            frag_ptr& fm = nbuf[*its].fptr;
            fbuf[ptr].ptr = fm.ptr;
            fbuf[ptr].off = fm.off;
            if ( idx == fm.ptr)
            {
                li.at(fm.off).isVoid = true;
            }
            else
                ptr++;
        }
        return ptr;
    }
};

#endif