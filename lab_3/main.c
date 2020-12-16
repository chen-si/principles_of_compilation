#include "RemoveLeftRecursion.h"
#include <string.h>
#include <stdlib.h>

const char* VoidSymbol = "$"; // "ε"
const char* Postfix = "'";

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
	// 输出消除左递归之前的文法
	//
	printf("Before Remove Left Recursion:\n");
	PrintRule(pHead);

	//
	// 调用 RemoveLeftRecursion 函数消除文法中的左递归
	//
	RemoveLeftRecursion(pHead);
	
	//
	// 输出消除左递归之后的文法
	//
	printf("\nAfter Remove Left Recursion:\n");
	PrintRule(pHead);
	
	return 0;
}

/*
功能：
	判断当前 Rule 中的一个 Symbol 是否需要被替换。
	如果 Symbol 是一个非终结符，且 Symbol 对应的
	Rule 在当前 Rule 之前，就需要被替换。

参数：
	pCurRule -- 当前 Rule 的指针。
	pSymbol -- Symbol 指针。
	
返回值：
	需要替换返回 1。
	不需要替换返回 0。
*/
int SymbolNeedReplace(const Rule* pCurRule, const RuleSymbol* pSymbol)
{
	// 是终结符，不需要替换
	if (pSymbol -> isToken){
		return 0;
	}
	
	Rule * pRule;
	for (pRule = pCurRule; pRule != NULL; pRule = pRule -> pNextRule ){
		// 在指向当前rule或当前rule之后，不需要替换
		if (pRule == pSymbol -> pRule){
			return 0;
		}
	}
	return 1;
}

/*
功能：
	拷贝一个 Symbol。

参数：
	pSymbolTemplate -- 需要被拷贝的 Symbol 指针。
	
返回值：
	拷贝获得的新 Symbol 的指针。
*/
RuleSymbol* CopySymbol(const RuleSymbol* pSymbolTemplate)
{
	// 分配内存给一个新的Symbol
	RuleSymbol* pNewSymbol = (RuleSymbol *)malloc(sizeof(RuleSymbol));
	
	pNewSymbol -> pNextSymbol = pSymbolTemplate -> pNextSymbol;
	pNewSymbol -> pOther = pSymbolTemplate -> pOther;
	pNewSymbol -> isToken = pSymbolTemplate -> isToken;
	strcpy(pNewSymbol -> TokenName, pSymbolTemplate -> TokenName);
	pNewSymbol -> pRule = pSymbolTemplate -> pRule;
	
	return pNewSymbol;
}

/*
功能：
	拷贝一个 Select。

参数：
	pSelectTemplate -- 需要被拷贝的 Select 指针。
	
返回值：
	拷贝获得的新 Select 的指针。
*/
RuleSymbol* CopySelect(const RuleSymbol* pSelectTemplate)
{
	// copy第一个symbol
	RuleSymbol* pNewSelect = CopySymbol(pSelectTemplate);
	RuleSymbol* pTempSelect, * pTempSymbol;
	
	// 链式copy所有symbol
	for (pTempSelect = pNewSelect, pTempSymbol = pSelectTemplate; pTempSymbol -> pNextSymbol != NULL; pTempSelect = pTempSelect -> pNextSymbol, pTempSymbol = pTempSymbol -> pNextSymbol){
		pTempSelect -> pNextSymbol = CopySymbol(pTempSymbol -> pNextSymbol);
	}
	return pNewSelect;
}

/*
功能：
	替换一个 Select 的第一个 Symbol。

参数：
	pSelectTemplate -- 需要被替换的 Select 指针。
	
返回值：
	替换后获得的新 Select 的指针。
	注意，替换后可能会有一个新的 Select，
	也可能会有多个 Select 链接在一起。
*/
RuleSymbol* ReplaceSelect(const RuleSymbol* pSelectTemplate)
{
	// 需要替换成的Rule
	Rule* pRule = pSelectTemplate -> pRule;
	// 新的Select
	RuleSymbol* pNewSelect = CopySelect(pRule -> pFirstSymbol);
	RuleSymbol* pTempSelect, * pTempSymbol, * pCurSelect;
	// 链式copy所有select
	for (pCurSelect = pNewSelect, pTempSelect = pRule -> pFirstSymbol; pTempSelect -> pOther != NULL; pCurSelect = pCurSelect -> pOther, pTempSelect = pTempSelect -> pOther){
		pCurSelect -> pOther = CopySelect(pTempSelect -> pOther);
	}
	
	// 
	for (pTempSelect = pNewSelect; pTempSelect != NULL; pTempSelect = pTempSelect -> pOther){
		for (pTempSymbol = pTempSelect; pTempSymbol -> pNextSymbol != NULL; pTempSymbol = pTempSymbol -> pNextSymbol){}
		pTempSymbol -> pNextSymbol = CopySymbol(pSelectTemplate -> pNextSymbol);
	}
	
	return pNewSelect;
}

/*
功能：
	释放一个 Select 的内存。

参数：
	pSelect -- 需要释放的 Select 的指针。
*/
void FreeSelect(RuleSymbol* pSelect)
{
	RuleSymbol* pSymbol = pSelect;
	while (pSymbol != NULL){
		RuleSymbol* pTemp = pSymbol;
		pSymbol = pSymbol -> pNextSymbol;
		free(pTemp);
	}
}
/*
功能：
	释放一个 Rule 链的内存。
参数：
	pRule -- 需要释放的 Rule 的指针。
*/
void FreeRule(Rule* pRule){
	while (pRule != NULL){
		RuleSymbol* pSelect = pRule -> pFirstSymbol;
		while (pSelect != NULL){
			// 释放pSelect指向的内存
			Rule* pTemp = pRule;
			pSelect = pSelect -> pOther;
			FreeSelect(pTemp);
		}
		// 释放当前Rule的内存
		Rule* pTemp = pRule;
		pRule = pRule -> pNextRule;
		free(pTemp);
	}
}

/*
功能：
	判断一条 Rule 是否存在左递归。

参数：
	prRule -- Rule 指针。

返回值：
	存在返回 1。
	不存在返回 0。
*/
int RuleHasLeftRecursion(Rule* pRule)
{
	RuleSymbol* pCurSelect = pRule -> pFirstSymbol;
	while(pCurSelect != NULL){
		// 如果当前Select的第一个symbol是当前的Rule，说明有左递归
		if (pCurSelect -> isToken == 0 && pCurSelect -> pRule == pRule){
			return 1;
		}
		pCurSelect = pCurSelect -> pOther;
	}
	
	return 0;
}

/*
功能：
	将一个 Symbol 添加到 Select 的末尾。

参数：
	pSelect -- Select 指针。
	pNewSymbol -- Symbol 指针。
*/
void AddSymbolToSelect(RuleSymbol* pSelect, RuleSymbol* pNewSymbol)
{
	RuleSymbol* pTempSymbol;
	// 让临时指针移动到select的symbol列表尾部
	for(pTempSymbol = pSelect; pTempSymbol -> pNextSymbol != NULL; pTempSymbol = pTempSymbol -> pNextSymbol){}
	// 将Symbol添加到Select尾部
	pTempSymbol -> pNextSymbol = pNewSymbol;
}

/*
功能：
	将一个 Select 加入到文法末尾，当 Select 为 NULL 时就将一个ε终结符加入到文法末尾。

参数：
	pRule -- 文法指针。
	pNewSelect -- Select 指针。
*/
void AddSelectToRule(Rule* pRule, RuleSymbol* pNewSelect)
{
	RuleSymbol* pTempSelect = pRule -> pFirstSymbol;
	if (pTempSelect == NULL){
		pRule -> pFirstSymbol = pNewSelect;
		return; 
	}
	// 让临时指针移动到select链表尾部
	while (pTempSelect -> pOther != NULL){
		pTempSelect = pTempSelect -> pOther;
	}
	// 如果是NULL则添加空字符终结符
	if (pNewSelect == NULL){
		pNewSelect = (RuleSymbol*)malloc(sizeof(RuleSymbol));
		pNewSelect -> pNextSymbol = NULL;
		pNewSelect -> pOther = NULL;
		pNewSelect -> isToken = 1;
		strcpy(pNewSelect -> TokenName, VoidSymbol);
	}
	// 将新的select添加到Rule末尾
	pTempSelect -> pOther = pNewSelect;
}

/*
功能：
	消除左递归。
参数：
	pHead -- 文法链表的头指针。
*/
void RemoveLeftRecursion(Rule* pHead){
	Rule* pRule;				// Rule 游标
	RuleSymbol* pSelect; 		// Select 游标
	Rule* pNewRule;			  	// Rule 指针
	int isChange;				// Rule 是否被替换的标记
	RuleSymbol **pSelectPrePtr; // Symbol 指针的指针
	for(pRule = pHead; pRule != NULL; pRule = pRule->pNextRule){
		// 替换
		do{
			isChange = 0;
			// 在 Rule 的所有 Select 中查找是否需要替换
			for(pSelect = pRule->pFirstSymbol, pSelectPrePtr = &pRule->pFirstSymbol;
				pSelect != NULL;
				pSelectPrePtr = &pSelect->pOther, pSelect = pSelect->pOther){
				if(SymbolNeedReplace(pRule, pSelect)) {// 判断 Select 的第一个 Symbol 是否需要替换
					isChange = 1;
					// 调用 ReplaceSelect 函数，替换 Select 的第一个 Symbol 后得到新的 Selects
					RuleSymbol* pNewSelects = ReplaceSelect(pSelect);
					// 使用新的 Selects 替换原有的 Select，并调用 FreeSelect 函数释放原有的 Select 内存
					RuleSymbol* pTempSelect = pNewSelects;
					// 找到新的 Selects 最后一个 select
					while(pTempSelect -> pOther != NULL){
						pTempSelect = pTempSelect -> pOther;
					} 
					// 链接末尾
					pTempSelect -> pOther = pSelect -> pOther;
					// 前一个 select 末尾连接新的 selects								
					*pSelectPrePtr = pNewSelects;
					pTempSelect = pSelect;
					// 使pSelect指向当前Select
					pSelect = pNewSelects;
					// 释放内存
					FreeSelect(pTempSelect);
					break;
				}
				if(isChange)
					break;
			}
		}while(isChange);
		// 忽略没有左递归的 Rule;
		if(!RuleHasLeftRecursion(pRule))
			continue;
		// 消除左递归
		pNewRule = CreateRule(pRule->RuleName); // 创建新 Rule
		strcat(pNewRule->RuleName, Postfix);
		pSelect = pRule->pFirstSymbol; // 初始化 Select 游标
		pSelectPrePtr = &pRule->pFirstSymbol;
		while(pSelect != NULL) {// 循环处理所有的 Select
			if(0 == pSelect->isToken && pSelect->pRule == pRule) { // Select 存在左递归
				// 移除包含左递归的 Select，将其转换为右递归后添加到新 Rule 的末尾，并移动游标
				// 创建一个select
				RuleSymbol* pTempSelect = CopySelect(pSelect -> pNextSymbol), * NewSymbol = CreateSymbol(),* pTemp;
				// 将前一个select的指针指向后一个select （删除当前select）
				*pSelectPrePtr = pTempSelect;
				// 将NewSymbol指向NewRule
				NewSymbol -> isToken = 0;
				NewSymbol -> pRule = pNewRule;
				// 将指向新rule的symbol加入select中
				AddSymbolToSelect(pTempSelect, NewSymbol);
				pTempSelect -> pOther = NULL;
				// 将当前select加入新的rule
				AddSelectToRule(pNewRule, pTempSelect);
				pTemp = pSelect;
				// 移动指针
				*pSelectPrePtr = pSelect -> pOther;
				pSelect = pSelect -> pOther;
				// 释放内存
				free(pTemp);
			}else { // Select 不存在左递归
				// 在没有左递归的 Select 末尾添加指向新 Rule 的非终结符，并移动游标
				// 创建一个指向新终结符的Symbol
				RuleSymbol* pNewSymbol = CreateSymbol();
				pNewSymbol -> isToken = 0;
				pNewSymbol -> pRule = pNewRule;
				// 加入到当前select末尾
				AddSymbolToSelect(pSelect, pNewSymbol);
				pSelectPrePtr = &pSelect -> pOther;
				pSelect = pSelect -> pOther;
			}
		}
		// 在新 Rule 的最后加入ε(用 '$' 代替)
		// 将新 Rule 插入文法链表
		AddSelectToRule(pNewRule, NULL);
		pNewRule -> pNextRule = pRule -> pNextRule;
		pRule -> pNextRule = pNewRule;
		pRule = pNewRule;
	}
}

/*
功能：
	使用给定的数据初始化文法链表

返回值：
	Rule 指针
*/
typedef struct _SYMBOL
{
	int isToken;
	char Name[MAX_STR_LENGTH];
}SYMBOL;

typedef struct _RULE_ENTRY
{
	char RuleName[MAX_STR_LENGTH];
	SYMBOL Selects[64][64];
}RULE_ENTRY;

static const RULE_ENTRY rule_table[] =
{
	/* A -> Ba | Aa | c */
	{ "A", 
			{
				{ { 0, "B" }, { 1, "a"} },
				{ { 0, "A" }, { 1, "a"} },
				{ { 1, "c" } }
			}	
	},

	/* B -> Bb | Ab | d */
	{ "B", 
			{
				{ { 0, "B" }, { 1, "b"} },
				{ { 0, "A" }, { 1, "b"} },
				{ { 1, "d" } }
			}	
	}
};

/*
功能：
	初始化文法链表。
	
返回值：
	文法的头指针。
*/
Rule* InitRules()
{
	// 初始化操作
	Rule *pHead, *pRule;
	RuleSymbol **pSymbolPtr1, **pSymbolPtr2;
	// 计算rule的数量
	int nRuleCount = sizeof(rule_table) / sizeof(rule_table[0]);
	int i, j, k;

	// 指向Rule的头指针
	Rule** pRulePtr = &pHead;
	// 初始化对应数量的rule
	for (i=0; i < nRuleCount; i++)
	{
		// 根据name创建rule
		*pRulePtr = CreateRule(rule_table[i].RuleName);
		// 移动rule指针
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	// 初始化pRule，方便遍历
	pRule = pHead;
	for (i=0; i<nRuleCount; i++)
	{
		// 设置对应的symbol
		pSymbolPtr1 = &pRule->pFirstSymbol;
		// 遍历初始化rule_table中定义好的所有select
		for (j=0; rule_table[i].Selects[j][0].Name[0] != '\0'; j++)
		{
			pSymbolPtr2 = pSymbolPtr1;
			// 遍历当前selec中的所有symbol
			for (k=0; rule_table[i].Selects[j][k].Name[0] != '\0'; k++)
			{
				// 存储symbol
				const SYMBOL* pSymbol = &rule_table[i].Selects[j][k];

				*pSymbolPtr2 = CreateSymbol();
				(*pSymbolPtr2)->isToken = pSymbol->isToken;
				// 是终结符
				if (1 == pSymbol->isToken)
				{
					// 直接将名字复制到对应symbol的tokenname中
					strcpy((*pSymbolPtr2)->TokenName, pSymbol->Name);
				}
				else
				{	// 不是终结符则指向对应的Rule
					(*pSymbolPtr2)->pRule = FindRule(pHead, pSymbol->Name);
					// 如果没有找到对应的rule
					if (NULL == (*pSymbolPtr2)->pRule)
					{
						printf("Init rules error, miss rule \"%s\"\n", pSymbol->Name);
						exit(1);
					}
				}
				// 移动到下一个symbol进行操作
				pSymbolPtr2 = &(*pSymbolPtr2)->pNextSymbol;
			}
			// 移动到下一个Select
			pSymbolPtr1 = &(*pSymbolPtr1)->pOther;
		}
		// 移动到下一条rule
		pRule = pRule->pNextRule;
	}

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
	RuleSymbol **pSymbolPtr1, **pSymbolPtr2;
		
	int i, j, k;

	Rule** pRulePtr = &pHead;
	for (i=0; i<nRuleCount; i++)
	{
		*pRulePtr = CreateRule(ruleNameArr[i]);
		pRulePtr = &(*pRulePtr)->pNextRule;
	}

	pRule = pHead;
	for (i=0; i<nRuleCount; i++)
	{
		pSymbolPtr1 = &pRule->pFirstSymbol;
		
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
			
		for (k = start; rule_table_ci[i][k] != '\0'; k++)
		{
			if (rule_table_ci[i][k] == '|')
			{
				pSymbolPtr1 = &(*pSymbolPtr1)->pOther;
				pSymbolPtr2 = pSymbolPtr1;
				continue;
			}
			if (rule_table_ci[i][k] == ' ')
			{
				continue;
			}
			if (k == start)
			{
				pSymbolPtr2 = pSymbolPtr1;
			}

			*pSymbolPtr2 = CreateSymbol();
			
			char tokenName[MAX_STR_LENGTH] = {};
			tokenName[0] = rule_table_ci[i][k];
			tokenName[1] = '\0';
			(*pSymbolPtr2)->isToken = 1;
			for (int m = 0; m < nRuleCount; m++)
			{
				if (strcmp(tokenName, ruleNameArr[m]) == 0)
				{
					(*pSymbolPtr2)->isToken = 0;
					(*pSymbolPtr2)->pRule = FindRule(pHead, tokenName);
					if (NULL == (*pSymbolPtr2)->pRule)
					{
						printf("Init rules error, miss rule \"%s\"\n", tokenName);
						exit(1);
					}
				}
			}
			if ((*pSymbolPtr2)->isToken == 1)
			{
				strcpy((*pSymbolPtr2)->TokenName, tokenName);
			}
			
			pSymbolPtr2 = &(*pSymbolPtr2)->pNextSymbol;
			
		}
			
		pRule = pRule->pNextRule;
	}

	return pHead;
}

/*
功能：
	创建一个新的 Rule。

参数：
	pRuleName -- 文法的名字。
	
返回值：
	Rule 指针
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
	创建一个新的 Symbol。
	
返回值：
	RuleSymbol 指针	
*/
RuleSymbol* CreateSymbol()
{
	RuleSymbol* pSymbol = (RuleSymbol*)malloc(sizeof(RuleSymbol));

	pSymbol->pNextSymbol = NULL;
	pSymbol->pOther = NULL;
	pSymbol->isToken = -1;
	pSymbol->TokenName[0] = '\0';
	pSymbol->pRule = NULL;

	return pSymbol;
}

/*
功能：
	根据 RuleName 在文法链表中查找名字相同的文法。

参数：
	pHead -- 文法的头指针。
	RuleName -- 文法的名字。
	
返回值：
	Rule 指针	
*/
Rule* FindRule(Rule* pHead, const char* RuleName)
{
	Rule* pRule;
	for (pRule = pHead; pRule != NULL; pRule = pRule->pNextRule)
	{
		if (0 == strcmp(pRule->RuleName, RuleName))
		{
			break;
		}
	}
	
	return pRule;
}	

/*
功能：
	输出文法。

参数：
	pHead -- 文法的头指针。
*/
void PrintRule(Rule* pHead){
	Rule* pRule = pHead;
	RuleSymbol* pRuleSymbol1,* pRuleSymbol2;
	// 遍历文法
	while(pRule != NULL){
		// 输出文法名
		printf("%s->", pRule -> RuleName);
		pRuleSymbol1 = pRule -> pFirstSymbol;
		// 遍历文法的每一个select
		while (pRuleSymbol1 != NULL){
			pRuleSymbol2 = pRuleSymbol1;
			// 遍历每一个symbol 
			while(pRuleSymbol2 != NULL){
				// 如果是终结符
				if (pRuleSymbol2 -> isToken == 1){
					printf("%s", pRuleSymbol2 -> TokenName);
				}else{
					printf("%s", pRuleSymbol2 -> pRule -> RuleName);
				}
				pRuleSymbol2 = pRuleSymbol2 -> pNextSymbol;
			}
			pRuleSymbol1 = pRuleSymbol1 -> pOther;
			// 如果还有select
			if (pRuleSymbol1 != NULL){
				printf("|");
			}else{
				printf("\n");
			}
		}
		pRule = pRule -> pNextRule;
	}	
}

/*
功能：
	释放DFA占用的内存

参数：
	dfa -- DFA的指针。
*/
void FreeDFA(DFA* dfa){
	int i;
	Transform* pTransform;
	Transform* nxt;
	for (i = 0; i < dfa->length; i++){
		// 遍历当前 DFA 状态的所有转换并清除
		for (pTransform = dfa->DFAlist[i]->firstTran; pTransform != NULL; pTransform = nxt){
			nxt = pTransform->NextTrans;
			free(pTransform);
		}
		// 清除当前 DFA 状态	
		free(dfa->DFAlist[i]);
	}
	// 清除 DFA 状态列表
	free(dfa);
}