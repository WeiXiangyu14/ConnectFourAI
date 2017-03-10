#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <windows.h>
#include "Point.h"
#include "Strategy.h"
#include "Judge.h"
#include "Node.h"

using namespace std;

const double c = 0.8;//参数
const double timeLimit = 4;//时间限制
const int sobig = 1000000;//一个大数，比较大小时用
const long long numLimit = 800000;//节点数量上限
int Mm=0;//棋盘大小的参数
int Nn=0;
int noXx=0;//不可落子点的位置
int noYy=0;
int Lx=0;
int Ly=0;
int topp[12];//当前列顶信息
int boardd[12][12];//当前棋盘信息
int nodePos = 0;//当前节点在数组中的位置
Node nodes[820000];//用数组存储蒙特卡洛搜索树的节点

int UCTSearch();//UCT算法
bool IsEndPoint(int v);//判断节点nodes[v]是不是终止节点
bool AbleExpand(int v);//判断节点nodes[v]是否可扩展
int TreePolicy(int v);
int Expand(int v);
int BestChild(int v,double ct);
int DefaultPoicy(int vl);
void BackUp(int v,int d);
int DefeatedSoon(int s[][12],int* t);//判断有没有对方直接胜利的棋
int WinSoon(int s[][12],int* t);//判断有没有自己直接胜利的棋
bool BusyDying(int s[][12],int* t,int q);//判断自己的某步棋会不会直接导致对方在下一步获胜

/*
	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误
	
	input:
		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性
		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记
		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0
		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board
				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])
				board[x][y]表示第x行、第y列的点(从0开始计)
				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0
		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的
				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略
		noX, noY : 棋盘上的不可落子点(注:其实这里给出的top已经替你处理了不可落子点，也就是说如果某一步
				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，
				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,
				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)
		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点
	output:
		你的落子点Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		不要更改这段代码
	*/
	int x = -1, y = -1;//最终将你的落子点存到x,y中
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}

	
	Nn=N;
	Mm=M;
	noXx=noX;
	noYy=noY;
	Lx=lastX;
	Ly=lastY;
	srand(time(0));

//	AllocConsole();


	for(int i = 0; i < N; i++)
		topp[i]=top[i];

	for(int i = 0; i < M; i++){
		for(int j = 0; j < N; j++){
			boardd[i][j] = board[i][j];
		}
	}

	bool findSolution = true;//判断UCT算法得到的结果是否是合法落子
	
    y=UCTSearch();
    if(y < 0 || y >= N)
    	findSolution = false;

    x=top[y]-1;
    if(x < 0 || y >= M)
    	findSolution = false;

	nodePos=0;

	if(findSolution == false)//以防万一，若出现非法落子，则返回一个虽愚蠢但合法的解
		{
			for (int i = N-1; i >= 0; i--) 
			{
				if (top[i] > 0) 
				{
					x = top[i] - 1;
					y = i;
					break;
				}
			}
		}
	
    
	
	
	/*
		不要更改这段代码
	*/
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint函数返回的Point指针是在本dll模块中声明的，为避免产生堆错误，应在外部调用本dll中的
	函数来释放空间，而不应该在外部直接delete
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p){
	delete p;
	return;
}

/*
	清除top和board数组
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


int UCTSearch()
{
	int DS=DefeatedSoon(boardd,topp);//判断有没有对方下一步直接胜利的棋
	int WS=WinSoon(boardd,topp);//判断有没有自己直接胜利的棋
	if( WS != -1)
		return WS;
	else if(DS != -1)
		return DS;

	nodes[0].father = -1;
	nodes[0].reset(Mm,Nn,boardd,topp,1);//nodes[0]存储当前棋盘的信息

	for(int i=0;i<Nn;i++)
	{
		if(BusyDying(boardd,topp,i))
			nodes[0].children[i] = -3;
	}

	int vl=-1;
	int delta=0;
	int endtime=0;


	clock_t start,now;
	start=clock();
	now = clock();
	double runtime=0; 


	while(nodePos<numLimit && runtime<timeLimit)
	{
		vl = TreePolicy(0);
		if(nodes[vl].ended != 2)
		{
			delta = nodes[vl].ended;
		}
		else
		{
			delta = DefaultPoicy(vl);
			
		}
		BackUp(vl,delta);

		endtime++;
		now = clock();
		runtime = (double)(now-start)/CLOCKS_PER_SEC;
	}
	
	vl = BestChild(0,0);


	return nodes[vl].a;
}

bool IsEndPoint(int v)
{
	return nodes[v].ended != 2;
}

bool AbleExpand(int v)
{
	bool res=false;
	for(int i=0;i<Nn;i++)
		if(nodes[v].children[i] == -1 && nodes[v].vtop[i] > 0)
			res = true;

	return res;
}

int TreePolicy(int v)
{
	while( !IsEndPoint(v) )
	{
		if( AbleExpand(v) )
		{
			return Expand(v);
		}
		else
			v = BestChild(v,c);
	}
return v;
}

int Expand(int v)
{
	for(int i=0;i<Nn;i++)
	{
		if(nodes[v].children[i] == -1 && nodes[v].vtop[i] > 0)
		{
			nodePos++;
			nodes[nodePos].father=v;
			nodes[nodePos].reset(Mm,Nn,nodes[v].s,nodes[v].vtop,3-nodes[v].side);

			nodes[nodePos].SetChess(i,Mm,Nn, noXx , noYy);
	
			nodes[v].children[i]=nodePos;
			return nodePos;
		}
	}
}

int BestChild(int v,double ct)
{
	double maxbc = -sobig;
	double tmp=0;
	int maxi=0;
	for(int i=0;i<Nn;i++)
	{
		if(nodes[v].children[i] > -1)
		{
			tmp = nodes[nodes[v].children[i]].setBC(ct , nodes[v].Nv);
			if(tmp>maxbc)
			{
				maxbc=tmp;
				maxi=i;
			}
		}
	}

	return nodes[v].children[maxi];
}

int DefaultPoicy(int v)
{
	int finishState = 2;

	int* tt=new int[Nn];
	for(int i=0;i<Nn;i++)
	{
		tt[i]=nodes[v].vtop[i];
	}

	int dd [12][12];
	for(int i = 0; i < Mm; i++)
	{
		for(int j = 0; j < Nn; j++)
		{
			dd[i][j] = nodes[v].s[i][j];
		}
	}
	int wsd=nodes[v].side;

	while(finishState == 2)
	{
		int ra=rand()%Nn;
		while(tt[ra]<=0)
		{
			ra--;
			if(ra<0)
				ra=Nn-1;
		}
		dd[tt[ra]-1][ra]=wsd;
		
		tt[ra]--;
		if(wsd==1)
			if(userWin(tt[ra],ra,Mm,Nn,dd)) 
			{
				if(nodes[v].side == 2)//刚下完的是2，所以该user下了
					finishState = -1;
				else
					finishState = 1;
			}
		if(wsd==2)
		{
			if(machineWin(tt[ra],ra,Mm,Nn,dd))
			{
				if(nodes[v].side == 2)//刚下完的是2，所以该user下了
					finishState = 1;
				else
					finishState = -1;
			} 
		}	
		if(finishState==2)
			if(isTie(Nn,tt))
				finishState = 0;

		if(noXx==tt[ra]-1 && noYy==ra)
			tt[ra]--;

		wsd=3-wsd;
	}
	
	delete tt;

	return finishState;
}

void BackUp(int v,int d)
{
	int tv=v;
	int td=d;
	while(tv!=-1)
	{
		nodes[tv].Nv++;
		nodes[tv].Q += td;
		td=-td;
		tv=nodes[tv].father;
	}
}

int DefeatedSoon(int s[][12],int* t)
{
	int ds=-1;
	int dd[12][12];
	for(int i = 0; i < Mm; i++)
	{
		for(int j = 0; j < Nn; j++)
		{
			dd[i][j] = s[i][j];
		}
	}
	int* tt=new int[Nn];
	for(int i=0;i<Nn;i++)
		tt[i]=t[i];

	int i=0;
	while(ds == -1 && i < Nn)
	{
		if(tt[i]>0)
		{
			dd[tt[i]-1][i] = 1;

			if(userWin(tt[i]-1,i,Mm,Nn,dd))
				ds=i;

			dd[tt[i]-1][i] = 0;
		}
		i++;
	}

	delete tt;

return ds;
}

int WinSoon(int s[][12],int* t)
{
	int ws=-1;
	int dd[12][12];
	for(int i = 0; i < Mm; i++)
	{
		for(int j = 0; j < Nn; j++)
		{
			dd[i][j] = s[i][j];
		}
	}
	int* tt=new int[Nn];
	for(int i=0;i<Nn;i++)
		tt[i]=t[i];

	int i=0;
	while(ws == -1 && i < Nn)
	{
		if(tt[i]>0)
		{
			dd[tt[i]-1][i] = 2;

			if(machineWin(tt[i]-1,i,Mm,Nn,dd))
				ws=i;

			dd[tt[i]-1][i] = 0;
		}
		i++;
	}
	delete tt;

return ws;
}

bool BusyDying(int s[][12],int* t,int q)//判断在q下棋是不是找死
{
	bool BD=false;

	if(t[q] <= 0)
		return false;

	int dd[12][12];
	for(int i = 0; i < Mm; i++)
		for(int j = 0; j < Nn; j++)
			dd[i][j] = s[i][j];
	int* tt=new int[Nn];
	for(int i=0;i<Nn;i++)
		tt[i]=t[i]; 


	dd[tt[q]-1][q] = 2;//模拟在q下一步棋
	tt[q]--;
	if(noXx==tt[q]-1 && noYy==q)
		tt[q]--;

	int i=0;
	while(BD==false && i<Nn)
	{
		if(tt[i]>0)
		{
			dd[tt[i]-1][i] = 1;

			if(userWin(tt[i]-1,i,Mm,Nn,dd))
				BD = true;

			dd[tt[i]-1][i] = 0;
		}
		i++;
	}
	delete tt;
return BD;
}