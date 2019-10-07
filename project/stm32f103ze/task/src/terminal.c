/********************************************************
*file name: terminal.c
*function: �����û������ָ�ִ�У�ָ�����֧��5������
*ʹ��˵����
	1.�ⲿ�ӿ�
		terminal_init() 			�ն��ڴ��ʼ������
		terminal_input_predeal() 	���ڴ��ڽ����ж��У�ǰ̨����
		terminal_handler()			������ѭ��while(1)�У���̨����
	2.��������
		Tab 		��ʾ����ƥ�������Զ�ƥ��ָ��
		��			������ʷ����
		��			ִ����ʷ����
		Enter		�����������ʼִ��
		Backspace	ɾ��ǰһ���ַ�
	3.���ָ��
		�����������stFunTab�����У���������Ϊ��������ָ��������������
	4.����ʾ��
		ָ���� �ո� ����1 �ո� ����2 �ո� ����3 �س�
		eg��test 123 345 �س�
	5.��ֲ˵��
		���ⲿ�ӿڷ��ں��ʵ�λ��
		�ض���ʵ��printf��������
		ʵ�ִ��ڴ�ӡ�ַ��ĺ�������ģ����ʹ��serial1_put_char
		��Ӻ���ָ��ӳ���
********************************************************/

#include <string.h>
#include <stdio.h>
#include "terminal.h"
#include "intermediate_uart.h"
#include "debug.h"

//�ⲿָ�������
extern void timer_list_print(void);
extern void task_list_print(void);
extern void timer_test(void);
extern void task_test(void);
extern void task_list_pop(void);
//����ָ��ӳ����壬����Ϊ������������ӳ��������βθ���
const Function_map_t stFunTab[] = 
{
	[0] = {.func = timer_list_print, 	.pName = "timerprint", 	.byParamterNum = 0},
	[1] = {.func = task_list_print,  	.pName = "taskprint", 	.byParamterNum = 0},
	[2] = {.func = timer_test, 		 	.pName = "timertest", 	.byParamterNum = 1},
	[3] = {.func = task_test, 			.pName = "tasktest", 	.byParamterNum = 1},
	[4] = {.func = task_list_pop, 		.pName = "pop", 		.byParamterNum = 0},
};
#define STFUNTAB_SIZE		(sizeof(stFunTab) / sizeof(stFunTab[0]))

//�ڲ��ն�ģ��ṹ�嶨��
static Terminal_t stTerminal;

//�ڲ���������
static void searching_command(void);
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd);
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue);
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum);
static bool recv_semantic_analysis(void);

/*********************************************************/
//name: terminal_init
//function: �ն�ģ�������ʼ��
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
void terminal_init(void)
{
	uint8_t i;
	memset(&stTerminal, 0, sizeof(Terminal_t));
	stTerminal.eState = TERMINAL_IDEL;
	stTerminal.OutPutCallBack = serial1_put_char; //���ڴ�ӡ����
	//�����б���
	for(i = 0; i < STFUNTAB_SIZE; i++)
	{
		if(NULL == stFunTab[i].func)
		{
			printf("stFunTab[%u] fun is NULL\r\n", i);
			while(1);
		}
	}
}

/*********************************************************/
//name: output_string
//function: �����ַ���������
//input: const char *pStr	�ַ�����ʼ��ַ
//		 uint8_t byLen		�ַ�������
//output: None
//return: None
//note: None
/*********************************************************/
void output_string(const char *pStr, uint8_t byLen)
{
	TASK_ERROR("pointer is null", (pStr != NULL), return;);
	
	while(byLen--)
	{
		stTerminal.OutPutCallBack(*pStr++);
	}
}

/*********************************************************/
//name: terminal_input_predeal
//function: �ն�����Ԥ����
//input: ���ڶ�ȡ��������
//output: None
//return: None
//note: �ú������ڽ����ж��е���Ϊ��
/*********************************************************/
void terminal_input_predeal(uint8_t byData)
{

	//ready״̬��ֱ���˳����ȴ�����������������
	if(stTerminal.eState == TERMINAL_READY)
	{
		return;
	}
	//����ASCII���ַ��� ���账��
	if(byData > 127)
	{
		return;
	}
	
	//��������ַ����������ң�����
	if(stTerminal.bySpecialCharFlag)
	{
		stTerminal.bySpecialCharFlag++;
		
		switch(stTerminal.bySpecialCharFlag)
		{
			case 2:
			{
				//���������ַ��Ŀ�����
				if(91 == byData)
				{
					return;
				}
				else	//�������ַ�
				{
					//�˴�Ӧ�ò���esc�����Ĵ����źţ�����esc����Ӧ�ò���δ������ʵ�֣����˵�
					
					//�ָ������ַ������־��
					stTerminal.bySpecialCharFlag = 0;
					break;
				}
			}
			case 3:
			{
				if (65 <= byData && byData <= 68)
				{
					stTerminal.byCtrlType = CHAR_TO_SPECIAL(byData);
					stTerminal.eState = TERMINAL_READY;
				}
				stTerminal.bySpecialCharFlag = 0;
				return;
			}
			default:
			{
				stTerminal.bySpecialCharFlag = 0;
				return;
			}
		}
	}
	
	//����ASCII������ַ�
	if(byData < 32)
	{
		if(27 == byData)
		{
			stTerminal.bySpecialCharFlag = 1;
		}
		else	//����������ַ�
		{
			stTerminal.eState = TERMINAL_READY;
			stTerminal.byCtrlType = byData;
		}
	}
	else	//����ASCII���ӡ�ַ�
	{
		//�������������
		stTerminal.OutPutCallBack(byData);
		
		//���ջ���������ǿ���л����������״̬
		if(RECV_BUFF_MAX_SIZE == stTerminal.byRecvLen)
		{
			stTerminal.eState = TERMINAL_READY;
			stTerminal.byCtrlType = RECV_BUFF_OVERFLOW;
		}
		else
		{
			stTerminal.byRecvBuff[stTerminal.byRecvLen++] = byData;
			stTerminal.eState = TERMINAL_BUSY;
		}
	}
}

/*********************************************************/
//name: terminal_handler
//function: �ն���������������
//input: None
//output: None
//return: None
//note: �ú����ʺϷ��ں�̨�����е���
/*********************************************************/
void terminal_handler(void)
{
	//����ͬ���뻥�������
	if (TERMINAL_READY != stTerminal.eState)
	{
		return;
	}
	
	switch(stTerminal.byCtrlType)
	{
		case '\b':	//backspace ����
		{
			stTerminal.OutPutCallBack('\b');
			stTerminal.OutPutCallBack(' ');
			stTerminal.OutPutCallBack('\b');
			stTerminal.byRecvLen = stTerminal.byRecvLen ? stTerminal.byRecvLen - 1: 0;
			break;
		}
			
		case '\t':	//Tab ����
		{
			searching_command();
			break;
		}
		
		case '\r':	//Enter ����
		{
			printf("\r\n");
			recv_semantic_analysis();
			if(stTerminal.byRecvLen)
			{
				stTerminal.byRecvLast = stTerminal.byRecvLen;
				stTerminal.byRecvLen = 0;
			}
			break;
		}
		
		case INPUT_KEY_UP:	//������
		{
			if(stTerminal.byRecvLen == 0)
			{
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLast);
				stTerminal.byRecvLen = stTerminal.byRecvLast;
			}
			break;
		}
		
		case INPUT_KEY_DOWN: //������
		{
			if(stTerminal.byRecvLen == 0)
			{
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLast);
				stTerminal.byRecvLen = stTerminal.byRecvLast;
				
				printf("\r\n");
				if(false == recv_semantic_analysis())
				{
					printf("input error\r\n");
				}
				stTerminal.byRecvLast = stTerminal.byRecvLen;
				stTerminal.byRecvLen = 0;
			}
			break;
		}
		
		case RECV_BUFF_OVERFLOW:	//���������
		{
			printf("uart recv over flow\r\n");
			break;
		}
		default :
		{
			break;
		}
	}
	stTerminal.eState = TERMINAL_IDEL;
}

/*********************************************************/
//name: searching_command
//function: �����Ѿ���������ݲ�������
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
static void searching_command(void)
{
	uint8_t i = 0;
	uint8_t byNameLength;
	uint8_t byMatchIndex[STFUNTAB_SIZE] = {0};
	uint8_t byMatchNum = 0;
	bool bMatchFlag = true;
	char byChar;
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		byNameLength = strlen(stFunTab[i].pName);
		if(stTerminal.byRecvLen > byNameLength)
		{
			continue;
		}
		
		if (0 == strncmp(stFunTab[i].pName, stTerminal.byRecvBuff, stTerminal.byRecvLen))
		{
			//�洢ƥ����ַ�
			byMatchIndex[byMatchNum++] = i;
		}
	}
	
	if (byMatchNum)
	{
		if(byMatchNum > 1)
		{
			printf("\r\n");
			for (i = 0; i < byMatchNum; i++)
			{
				printf("%s   ", stFunTab[byMatchIndex[i]].pName);
			}
			printf("\r\n");
			
			//������ͬ���ַ����������add1 add2������a֮�󣬰�tab��ʾadd
			while(bMatchFlag)
			{
				byChar = stFunTab[byMatchIndex[0]].pName[stTerminal.byRecvLen];
				for (i = 1; i < byMatchNum; i++)
				{
					if (byChar != stFunTab[byMatchIndex[i]].pName[stTerminal.byRecvLen])
					{
						bMatchFlag = false;
						break;
					}
				}
				stTerminal.byRecvLen++;
			}
			
			strncpy(stTerminal.byRecvBuff, stFunTab[byMatchIndex[0]].pName, --stTerminal.byRecvLen);
			output_string(stTerminal.byRecvBuff, stTerminal.byRecvLen);
		}
		else
		{
			byNameLength = strlen(stFunTab[byMatchIndex[0]].pName);
			strncpy(stTerminal.byRecvBuff, stFunTab[byMatchIndex[0]].pName, byNameLength);
			stTerminal.byRecvBuff[byNameLength++] = ' '; //��ӿո��
			output_string(&stTerminal.byRecvBuff[stTerminal.byRecvLen], byNameLength - stTerminal.byRecvLen);
			stTerminal.byRecvLen = byNameLength;
		}
	}
}
/*********************************************************/
//name: recv_semantic_analysis
//function: ������������ݷ�������
//input: None
//output: None
//return: true���������	false���������
//note: None
/*********************************************************/
static bool recv_semantic_analysis(void)
{
	uint8_t i, byCmdLen;
	uint8_t byTemp;
	uint8_t byHead = 0, byTail = 0;
	uint32_t dwPramter[FUN_ARGUMENTS_MAX_SIZE] = {0};	 //����������
	uint8_t byParamterNum = 0;			//��������
	uint8_t byCmdIndex = STFUNTAB_SIZE;	//ƥ������λ��
	if (0 == stTerminal.byRecvLen)	//���ݶ�Ϊ��
	{
		return false;
	}
	
	byCmdLen = separate_string(&byHead, ' ', &byTail);
	if(0 == byCmdLen)
	{
		printf("input error\r\n");
		return false;
	}
	//��������
	for(i = 0; i < STFUNTAB_SIZE; i++)	
	{
		byTemp = strlen(stFunTab[i].pName);
		if(byCmdLen != byTemp)
		{
			continue;
		}
		
		if(0 == strncmp(stFunTab[i].pName, &stTerminal.byRecvBuff[byHead], byCmdLen))
		{
			byCmdIndex = i; //��¼�����λ��
			break;
		}
	}
	if (STFUNTAB_SIZE == byCmdIndex)	//δ�ҵ�����
	{
		printf("can not find that cmd\r\n");
		return false;
	}
	
	//��������Ĳ���
	for(i = byTail; i < stTerminal.byRecvLen; i += byTemp)
	{
		byHead = byTail + 1;
		byTemp = separate_string(&byHead, ' ', &byTail);
		if(0 == byTemp || byParamterNum == FUN_ARGUMENTS_MAX_SIZE)
		{
			break;
		}
		
		if(false == string_to_uint(&stTerminal.byRecvBuff[byHead], byTemp, &dwPramter[byParamterNum++]))
		{
			printf("paramater error\r\n");
			return false;
		}
	}
	
	if(byParamterNum != stFunTab[byCmdIndex].byParamterNum)
	{
		printf("paramter num not match\r\n");
		return false;
	}
	
	execute_handled(byCmdIndex, dwPramter, byParamterNum);
	return true;
}

/*********************************************************/
//name: separate_string
//function: ���ݷָ����Խ��յ����ݽ��зֶ�
//input: uint8_t *pStart �ָ���ʼλ��
//		 const char chr	�ָ���
//output: uint8_t *pEnd	�ַ����ָ�����λ��
//return: ƥ����ַ�������
//note: pEnd�Ľ���λ��Ϊ��һ���ָ���
/*********************************************************/
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd)
{
	uint8_t byLen = 0;
	TASK_ERROR("pointer is null", (pStart != NULL && pEnd != NULL), return false;);
	//���˶���ķָ���
	while(*pStart < stTerminal.byRecvLen)
	{
		if(stTerminal.byRecvBuff[*pStart] != chr)
		{
			break;
		}
		++(*pStart);
	}
	
	//�����ַ���
	*pEnd = *pStart;
	while(*pEnd < stTerminal.byRecvLen)
	{
		if(stTerminal.byRecvBuff[*pEnd] == chr)
		{
			break;
		}
		++(*pEnd);
	}
	return *pEnd - *pStart;
}

/*********************************************************/
//name: string_to_uint
//function: ���ַ���ת��Ϊ����
//input: char *pStr �ַ�����ʼ��ַ
//		 uint8_t byLen	�ַ�������
//output: uint32_t *pValue ת�������ֵ
//return: true �ַ���ת���ɹ�  false ת������
//note: ��֧��16λ���ƺ�10����������
/*********************************************************/
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue)
{
	uint32_t dwBaseValue = 10;
	uint32_t dwCharValue;
	bool bHexFlag = false;
	
	TASK_ERROR("pointer is null", (pStr != NULL && pValue != NULL), return false;);
	
	if('0' == pStr[0] && ('x' == pStr[1] || 'X' == pStr[1]))
	{
		if(byLen > 10)	//�������
		{
			return false;
		}
		byLen -= 2;
		pStr += 2;
		dwBaseValue = 16;
		bHexFlag = true;
	}
	
	*pValue = 0;
	
	if(false == bHexFlag)
	{
		while(byLen--)
		{
			if(*pStr > '9' || *pStr < '0')
			{
				return false;
			}
			*pValue *= dwBaseValue;
			*pValue += *pStr - '0';
			pStr++;
		}
	}
	else
	{
		while(byLen--)
		{
			//�������
			if(*pStr <= '9' && *pStr >= '0')
			{
				dwCharValue = *pStr - '0';
			}
			else if(*pStr <= 'f' && *pStr >= 'a')
			{
				dwCharValue = *pStr - 'a' + 10;
			}
			else if (*pStr <= 'F' && *pStr >= 'A')
			{
				dwCharValue = *pStr - 'A' + 10;
			}
			else
			{
				return false;
			}
			//�����ý��
			*pValue *= dwBaseValue;
			*pValue += dwCharValue;
			pStr++;
		}
	}

	return true;
}

/*********************************************************/
//name: execute_handled
//function: ִ�н���������
//input: uint8_t byFunIndex 	��Ҫ���еĺ����ں����б��λ��
//		 uint32_t *pArg	  		�����б�
//		 uint8_t byArgNum		��������
//output: uint32_t *pValue ת�������ֵ
//return: true �ַ���ת���ɹ�  false ת������
//note: ��֧��16λ���ƺ�10����������
/*********************************************************/
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum)
{
	switch(byArgNum)
	{
		case 0:
		{
			((void (*)())stFunTab[byFunIndex].func)();
			break;
		}
		case 1:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0]);
			break;
		}
		case 2:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1]);
			break;
		}
		case 3:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2]);
			break;
		}
		case 4:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3]);
			break;
		}
		case 5:
		{
			((void (*)())stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3], pArg[4]);
			break;
		}
	}
}
