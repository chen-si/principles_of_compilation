#include "RegexpToPost.h"
#include <string.h>

/*
���ܣ�
	�������������ʽ�ַ���ת���ɽ������ĺ����������С�

������
	re -- ������ʽָ�롣
	  
����ֵ��
	�������ĺ���������������ָ�롣
*/
char* re2post(char *re)
{
	int nalt; 	// ��ʾ����������ַ�Ϊֹ���Ѿ��ж��ٸ���֧�ṹ 
	int natom;	// ��ʾ����������ַ�Ϊֹ���Ѿ��ж��ٸ�ԭ�ӽṹ 
	static char buf[8000];
	char *dst;
	
	struct postseq
	{
		int nalt;
		int natom;
	} paren[100], *p;
	
	p = paren;
	dst = buf;
	nalt = 0;
	natom = 0;
	if (strlen(re) >= sizeof buf/2)
	{
		return NULL;
	}
		
	for (; *re; re++)
	{
		switch (*re)
		{
			case '(':
				if (natom > 1)
				{
					--natom;
					*dst++ = '.';
				}
				if (p >= paren+100)
				{
					return NULL;
				}
				p->nalt = nalt;
				p->natom = natom;
				p++;
				nalt = 0;
				natom = 0;
				break;
			case '|':
				if (natom == 0)
				{
					return NULL;
				}
				while (--natom > 0)
				{
					*dst++ = '.';
				}
				nalt++;
				break;
			case ')':
				if (p == paren)
				{
					return NULL;
				}
				if (natom == 0)
				{
					return NULL;
				}
				while (--natom > 0)
				{
					*dst++ = '.';
				}
				for (; nalt > 0; nalt--)
				{
					*dst++ = '|';
				}
				--p;
				nalt = p->nalt;
				natom = p->natom;
				natom++;
				break;
			case '*':
			case '+':
			case '?':
				if (natom == 0)
				{
					return NULL;
				}
				*dst++ = *re;
				break;
			default:
				if (natom > 1)
				{
					--natom;
					*dst++ = '.';
				}
				*dst++ = *re;
				natom++;
				break;
		}
	}
	if (p != paren)
	{
		return NULL;
	}
	while (--natom > 0)
	{
		*dst++ = '.';
	}
	for (; nalt > 0; nalt--)
	{
		*dst++ = '|';
	}
	*dst = 0;
	
	return buf;
}
