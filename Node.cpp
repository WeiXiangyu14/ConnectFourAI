#include "Node.h"

Node::Node()
{
	father = -2;

	for(int i = 0; i < 12; i++)
		for(int j = 0; j < 12; j++)
			s[i][j] = 0;

	for(int i=0;i<12;i++)
	{
		vtop[i] = 0;
		children[i] = -1;
	}

	a=0;
	Q=0;
	Nv=0;
	ended=2;
	BC=0;
}

Node::~Node()
{
//考虑需要删除数组吗？
}

void Node::reset(int m, int n, int b[12][12], int* t ,int sd)
{
	for(int i=0;i<n;i++)
		children[i] = -1;

	a=0;
	Q=0;
	Nv=0;
	ended=2;
	BC=0;

	for(int i=0;i<m;i++)
	{
		for(int j=0;j<n;j++)
			s[i][j]=b[i][j];
	}
	for(int i=0;i<n;i++)
		vtop[i]=t[i];
		
	side=sd;
	
	return;
}

double Node::setBC(double ct,int NofFather)
{
	double les = 0.0000001;

	BC = double(Q)/(double(Nv) + les) + ct*sqrt(2*log(double(NofFather))/(double(Nv)+les));//需要检查

	//_cprintf("setBC: Q=%d,Nv=%d,father->Nv=%d,BC=%lf\n",Q,Nv,father->Nv,BC);
	return BC;  
}

void Node::SetChess(int i,int m,int n,int nox,int noy)
{
	a=i;
	s[vtop[i]-1][i] = side;
	vtop[i]--;

	if(side==1)
	{
		if(userWin(vtop[i],i,m,n,s)) 
			ended = -1;
	}
	if(side==2)
	{
		if(machineWin(vtop[i],i,m,n,s)) 
			ended = 1;
	}
	if(ended==2)
		if(isTie(n,vtop))
			ended = 0;

	if(nox==vtop[i]-1 && noy==i)
		vtop[i]--;
}

void Node::showinfo()
{
//	for(int i=0;i<Mm;i++)
//	{
//		for(int j=0;j<Nn;j++)
	//		_cprintf("%d ",s[i][j]);
	//	_cprintf("\n");
////	}
//	_cprintf("vtop: ");	
//	for(int j=0;j<Nn;j++)
//		_cprintf("%d ",vtop[j]);
//	_cprintf("\n");

//	_cprintf("side = %d\n",side);
}