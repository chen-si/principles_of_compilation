#include "case.h"


int main(int argc, char* argv[])
{
	case1();
	case2();
	case3();
	case4();
	case5();
	case6();
	case7();
	case8();
	case9();
	case10();

	return 0;
}

void case1()
{
	int i = add(1, 2);
}

int __fastcall add(int i, int j)
{
	return i + j;
}

void case2()
{
	int i = 1;
	int a[4];
	a[0] = 6;
	a[1] = 7;
	a[i + 1] = 8;
}

typedef struct _Rec
{
	int i;
	char c;
	int j;
}Rec;

void case3()
{
	Rec x;
	x.j = 1;
	x.c = x.j;
	x.i = x.j;
}

typedef struct _TreeNode
{
	int val;
	struct _TreeNode *lchild, *rchild;
}TreeNode;

void case4()
{
	TreeNode* p = (TreeNode*)malloc(sizeof(TreeNode));
	p->lchild = p;
	p = p->rchild;
}

void case5()
{
	int x, y;
	if(x > y)
		y++;
	else
		x--;
}

void case6(){
	int i = 1;
	int a[3][3];
	a[0][0] = 6;
	a[1][1] = 7;
	a[1][i+1] = 8;
}

typedef struct _Cub{
	int i;
	union{
		char c;
		int j;
	}u;
}Cub;

void case7(){
	Cub x;
	x.i = 1;
	x.u.c = x.i;
	x.u.j = x.i;
}

void case8(){
	TreeNode* p = (TreeNode*)malloc(sizeof(TreeNode));
	TreeNode** pp = &p;
	(*pp) -> val = 1;
}

void case9(){
	int x,y;
	while (x < y){
		y -= x;
	}
}

void case10(){
	int x,y;
	x = 1;
	y = 3;
	x = ((x < y) || ((y = x > y) == 0));
}

