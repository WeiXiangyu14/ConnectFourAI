#ifndef NODE_H_
#define NODE_H_

#include <conio.h>
#include <atlstr.h>
#include "Point.h"
#include "Judge.h"
#include <iostream>
using namespace std;

class Node
{
public:
	Node();
	~Node();
	double setBC(double ct,int NofFather);
	void SetChess(int i,int m,int n,int nox,int noy);
	void showinfo();
	void reset(int m, int n, int b[12][12], int* t ,int sd);
public:
	int father;
	int children[12];
	int s[12][12];
	int vtop[12];
	int a;
	int Q;
	int Nv;
	int side;//该局面下，刚下完棋的一方
	int ended;
	double BC;
};

#endif