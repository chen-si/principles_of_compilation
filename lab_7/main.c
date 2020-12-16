#include "First.h"
#include <string.h>
#include <stdlib.h>

const char* VoidSymbol = "$"; // "ε"

char rule_table_ci[20][256];
char ruleNameArr[20][64];

int main(int argc, char* argv[])
{
	//
	// 调用 InitRules 函数初始化文法
	//
#ifdef CODECODE_CI
	Rule* pHead = InitRules_CI();  	// 此行代码在线上流水线运行
#else
	Rule* pHead = InitRules();		// 此行代码在 CP Lab 中运行
#endif
	
	//
	// 输出文法
	//
	PrintRule(pHead);
	
	//
	// 调用 First 函数求文法的 First 集合
	//
	SetList FirstSet;
	FirstSet.nSetCount = 0;
	First(pHead, &FirstSet);
	
	//
	// 输出First集合
	// 
	PrintFirstSet(&FirstSet);

	return 0;
}

/*
功能：
	添加一个 Set 到 SetList。

参数：
	pSetList -- SetList 指针。
	pName -- 集合名称字符串指针。 
*/
void AddOneSet(SetList* pSetList, const char* pName)
{	
	int found = 0;
	if (GetSet(pSetList, pName) != NULL){
		// 该name的set集合已经存在
		found = 1;
	}
	// 该set集合还不存在
	if (found == 0){
		Set* pSet = (Set *)malloc(sizeof(Set));
		strcpy(pSet -> Name, pName);
		pSet -> nTerminalCount = 0;
		// 添加set到setlist中
		pSetList -> Sets[pSetList -> nSetCount ++] = *pSet;
	}
}

/*
功能：
	根据名称在 SetList 中查找 Set。

参数：
	pSetList -- SetList 指针。
	pName -- 集合名称字符串指针。
返回值：
	如果找到返回 Set 的指针，否则返回 NULL。
*/
Set* GetSet(SetList* pSetList, const char* pName)
{
	int i;
	for (i = 0; i < pSetList -> nSetCount; i++){
		if (0 == strcmp(pSetList -> Sets[i].Name, pName)){
			return &pSetList -> Sets[i];
		}
	}	
	return NULL;
}

/*
功能：
	添加一个终结符到 Set。

参数：
	pSet -- Set 指针。
	pTerminal -- 终结符名称指针。
返回值：
	添加成功返回 1。
	添加不成功返回 0。
*/
int AddTerminalToSet(Set* pSet, const char* pTerminal)
{
	int found = 0, i;
	// 先排查重复的Terminal
	for (i = 0; i < pSet -> nTerminalCount; i++){
		if (0 == strcmp(pTerminal, pSet -> Terminal[i])){
			found = 1;
			return 0;
		}
	}
	// 无重复再添加进集合
	if (0 == found){
		strcpy(pSet -> Terminal[pSet -> nTerminalCount++], pTerminal);
		return 1;
	}
}

/*
功能：
	将源 Set 中的所有终结符添加到目标 Set 中。

参数：
	pDesSet -- 目标 Set 指针。
	pSrcSet -- 源 Set 指针。
返回值：
	添加成功返回 1，否则返回 0。
*/
int AddSetToSet(Set* pDesSet, const Set* pSrcSet)
{
	if (pDesSet == pSrcSet){
		return 0;
	}
	int i,j;
	int isChange = 0;
	for (i = 0; i < pSrcSet -> nTerminalCount; i++){
		int found = 0;
		for (j = 0; j < pDesSet -> nTerminalCount; j++){
			if (0 == strcmp(pDesSet -> Terminal[j], pSrcSet -> Terminal[i])){
				// 找到了重复项，直接跳出内循环
				found = 1;
				break;
			}
		}
		if ( 0 == found ){
			strcpy(pDesSet -> Terminal[pDesSet -> nTerminalCount ++], pSrcSet -> Terminal[i]);
			isChange = 1;
		}
	}
	return isChange;	
}

/*
功能：
	判断 Set 的终结符中是否含有ε。

参数：
	pSet -- Set 指针。
返回值：
	存在返回 1。
	不存在返回 0。
*/
int SetHasVoid(const Set* pSet)
{
	int i;
	for (i = 0; i < pSet -> nTerminalCount; i++){
		if (0 == strcmp(VoidSymbol, pSet -> Terminal[i])){
			return 1;
		}	
	}	
	return 0;
}

/*
功能：
	求文法的 First 集合。

参数：
	pHead -- 文法的头指针。
	pFirstSetList -- First 集合指针。
*/
void First(const Rule* pHead, SetList* pFirstSetList){
	const Rule* pRule;  // Rule 指针
	int isChange;	    // 集合是否被修改的标志
	RuleSymbol* pSymbol;// Symbol 游标
	// 使用文法链表初始化 First 集合
	for(pRule = pHead; pRule != NULL; pRule = pRule->pNextRule){
		AddOneSet(pFirstSetList, pRule->RuleName);
	}
	do{
		isChange = 0; // 设置修改标志
		for(pRule = pHead; pRule != NULL; pRule = pRule->pNextRule){
			// 根据文法名称在 pFirstSetList 中查找 Set
			Set* pDesSet = GetSet(pFirstSetList, pRule->RuleName);
			int hasVoid = 1; // First 集合中是否含有ε的标志
			for(pSymbol = pRule->pFirstSymbol; pSymbol != NULL && hasVoid; pSymbol = pSymbol->pNextSymbol){
				if(pSymbol->isToken) {	// 终结符
					// 调用 AddTerminalToSet 函数将终结符添加到 pDesSet，并设置修改标志
					if(AddTerminalToSet(pDesSet, pSymbol->SymbolName))
						isChange = 1;
					hasVoid = 0; // 设置 First 集合中是否含有ε的标志
				}else{	// 非终结符
					// 根据非终结符名称在 pFirstSetList 中查找 Set
					Set* pSrcSet = GetSet(pFirstSetList, pSymbol->SymbolName);
					// 调用 AddSetToSet 函数，将源 Set 中的所有终结符添加到目标 Set 中，并设置修改标志
					if(AddSetToSet(pDesSet, pSrcSet))
						isChange = 1;
					// 调用 SetHasVoid 函数，判断源 Set 中是否含有ε
					hasVoid = SetHasVoid(pSrcSet);
				}
			}
			if(hasVoid){// 调用 AddTerminalToSet 函数将ε加入到目标集合中
				AddTerminalToSet(pDesSet, VoidSymbol);
			}
		}
	} while(isChange);
}

typedef struct _SYMBOL
{
	int isToken;
	char SymbolName[MAX_STR_LENGTH];
}SYMBOL;

typedef struct _RULE_ENTRY
{
	char RuleName[MAX_STR_LENGTH];
	SYMBOL Symbols[64];
}RULE_ENTRY;

static const RULE_ENTRY rule_table[] =
{
	/* exp -> exp addop term| term */
	{ "exp", { { 0, "exp" }, { 0, "addop"}, { 0, "term"} } },
	{ "exp", { { 0, "term" } } },

	/* addop -> + | - */
	{ "addop", { { 1, "+" } } },
	{ "addop", { { 1, "-" } } },

	/* term -> term mulop factor | factor */
	{ "term", { { 0, "term" }, { 0, "mulop"}, { 0, "factor"} } },
	{ "term", { { 0, "factor" } } },

	/* mulop -> * */
	{ "mulop", { { 1, "*" } } },

	/* factor -> (exp) | number */
	{ "factor", { { 1, "(" }, { 0, "exp"}, { 1, ")"} } },
	{ "factor", { { 1, "number" } } },
};

/*
功能：
	初始化文法链表。
返回值：
	文法的头指针。
*/
Rule* InitRules(){
	Rule *pHead, *pRule;
	RuleSymbol **pSymbolPtr, *pNewSymbol;
	// 计算rule的数量
	int nRuleCount = sizeof(rule_table) / sizeof(rule_table[0]);
	int i, j;
	// 指向rule头指针的指针
	Rule** pRulePtr = &pHead;
	for (i=0; i<nRuleCount; i++){
		// 通过rule_table中的rulename创建新的rule
		*pRulePtr = CreateRule(rule_table[i].RuleName);
		// 移动rule指针，使他指向下一个rule指针的存储位置
		pRulePtr = &(*pRulePtr)->pNextRule;
	}
	pRule = pHead;
	for (i=0; i<nRuleCount; i++){
		// 指向第一个symbol指针的指针
		pSymbolPtr = &pRule->pFirstSymbol;
		for (j=0; rule_table[i].Symbols[j].SymbolName[0] != '\0'; j++){	
			// 常量存储rule_table中的symbol 
			const SYMBOL* pSymbol = &rule_table[i].Symbols[j];
			// 创建一个新的symbol
			pNewSymbol = CreateSymbol();
			// 将这个深copy rule_table 中对应的symbol给新symbol
			pNewSymbol->isToken = pSymbol->isToken;
			strcpy(pNewSymbol->SymbolName, pSymbol->SymbolName);
			// 连接新的symbol到rule
			*pSymbolPtr = pNewSymbol;
			// 移动symbol指针，使之指向下一个symbol的指针
			pSymbolPtr = &pNewSymbol->pNextSymbol;
		}
		// 移动rule指针
		pRule = pRule->pNextRule;
	}
	// 返回rule头指针
	return pHead;
}

/*
功能：
	初始化文法链表(在执行流水线时调用)。
	
返回值：
	文法的头指针。
*/
Rule* InitRules_CI()
{
	int nRuleCount = 0;
	// 需要读取多行文本
	for (int i = 0; i < 20; i++)
	{
		gets(rule_table_ci[i]);	
		int length = strlen(rule_table_ci[i]);
		if (length == 0)
		{
			break;
		}
		
		for (int j = 0; j < length; j++)
		{
			if (rule_table_ci[i][j] == ' ')
			{
				ruleNameArr[i][j] = '\0';
				break;
			}
			ruleNameArr[i][j]= rule_table_ci[i][j];
		}	  
		nRuleCount++;
	}
	
	Rule *pHead, *pRule;
	RuleSymbol **pSymbolPtr, *pNewSymbol;

	Rule** pRulePtr = &pHead;
	for (int i=0; i<nRuleCount; i++)
	{
		*pRulePtr = CreateRule(ruleNameArr[i]);
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	pRule = pHead;
	for (int i=0; i<nRuleCount; i++)
	{
		pSymbolPtr = &pRule->pFirstSymbol;
		
		int start = 0;
		for (int j=0; rule_table_ci[i][j] != '\0'; j++)
		{
			if (rule_table_ci[i][j] == ' '
			 && rule_table_ci[i][j + 1] == '-'
			&& rule_table_ci[i][j + 2] == '>' 
			&& rule_table_ci[i][j + 3] == ' ')
			{
				start = j + 4;
				break;
			}
		}
			
		for (int k=start; rule_table_ci[i][k] != '\0'; k++)
		{
			if (rule_table_ci[i][k] == ' ')
			{
				continue;
			}
				
			pNewSymbol = CreateSymbol();
			char tokenName[MAX_STR_LENGTH] = {};
			
			for (int m = 0; ;m++)
			{
				if (rule_table_ci[i][k] ==  ' ' || rule_table_ci[i][k] == '\0' || rule_table_ci[i][k] == '\n')
				{
					tokenName[m] = '\0';
					break;
				}
				tokenName[m] = rule_table_ci[i][k++];
				
			}
			
			
			strcpy(pNewSymbol->SymbolName, tokenName);
			
			pNewSymbol->isToken = 1;
			for (int n = 0; n < nRuleCount; n++)
			{
				if (strcmp(pNewSymbol->SymbolName, ruleNameArr[n]) == 0)
				{
					pNewSymbol->isToken = 0;
					break;
				}
			}		
			
			*pSymbolPtr = pNewSymbol;

			pSymbolPtr = &pNewSymbol->pNextSymbol;
			
		}
			
		pRule = pRule->pNextRule;
	}

	return pHead;
}

/*
功能：
	创建一个新的文法。
	
参数：
	pRuleName -- 文法的名字。	
	
返回值：
	文法的指针。
*/
Rule* CreateRule(const char* pRuleName)
{
	Rule* pRule = (Rule*)malloc(sizeof(Rule));

	strcpy(pRule->RuleName, pRuleName);
	pRule->pFirstSymbol = NULL;
	pRule->pNextRule = NULL;

	return pRule;
}

/*
功能：
	创建一个新的符号。
	
返回值：
	符号的指针。
*/
RuleSymbol* CreateSymbol()
{
	RuleSymbol* pSymbol = (RuleSymbol*)malloc(sizeof(RuleSymbol));

	pSymbol->pNextSymbol = NULL;
	pSymbol->isToken = -1;
	pSymbol->SymbolName[0] = '\0';

	return pSymbol;
}

/*
功能：
	输出文法。
	
参数：
	pHead -- 文法的头指针。
*/
void PrintRule(const Rule* pHead){
	Rule* pRule = pHead;
	RuleSymbol* pRuleSymbol;
	while(pRule != NULL){
		printf("%s -> ", pRule -> RuleName);// 输出RuleName
		pRuleSymbol = pRule -> pFirstSymbol;
		while(pRuleSymbol != NULL){
			// 打印symbol name
			printf("%s ",pRuleSymbol -> SymbolName);
			// 移动symbol执政
			pRuleSymbol = pRuleSymbol -> pNextSymbol;
		}
		pRule = pRule -> pNextRule;
		printf("\n");// 换行打印下一个文法
	}
}

/*
功能：
	输出 First 集合。

参数：
	pFirstSetList -- First 集合指针。
*/
void PrintFirstSet(SetList* pFirstSetList)
{
	printf("\nThe First Set:\n");
	for (int i = 0; i < pFirstSetList->nSetCount; i++)
	{
		printf("First(%s) = { ", pFirstSetList->Sets[i].Name);
		for (int j = 0; j < pFirstSetList->Sets[i].nTerminalCount; j++)
		{
			if (j == pFirstSetList->Sets[i].nTerminalCount - 1)
			{
				printf("%s ", pFirstSetList->Sets[i].Terminal[j]);
			}
			else
			{
				printf("%s, ", pFirstSetList->Sets[i].Terminal[j]);
			}
			
		}
		printf("}\n");
	}
}

/*
功能：
	释放 Rule 占用的内存。
参数：
	pRule -- Rule头执政。
*/
void FreeRule(Rule* pRule){
	while (pRule != NULL){// 释放Rule的内粗
		RuleSymbol* pRuleSymbol = pRule -> pFirstSymbol;
		while (pRuleSymbol != NULL){// 释放Rule对应的RuleSymbol的内存
			RuleSymbol* pTemp = pRuleSymbol;// 临时变量，辅助释放内存
			pRuleSymbol = pRuleSymbol -> pNextSymbol;
			free(pTemp);
		}
		Rule* pTemp = pRule;// 临时变量，辅助释放内存
		pRule = pRule -> pNextRule;
		free(pTemp);
	}
}

