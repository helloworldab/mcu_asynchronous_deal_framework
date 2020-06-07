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
#include "debug.h"

#define 	PUTCHAR_WITH_IT

//�ڲ���������
static void searching_command(void);
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd);
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue);
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum);
static bool recv_semantic_analysis(void);
static void input_echo(void);
static void output_string(const char *pStr, uint8_t byLen);
static bool check_login_cmd(uint8_t byIndex);

static void tester_login(uint32_t key);
static void admin_login(uint32_t key);
static void software_reset(void);
static void user_help(void);

//�ⲿָ�������
extern void timer_list_print(void);
extern void task_list_print(void);
extern void timer_test(void);
extern void task_test(void);
extern void task_list_pop(void);

//����ָ��ӳ����壬����Ϊ������������ӳ��������βθ���
//ָ����ڵײ��������Ŵ�����Ȩ�޵Ĵ�С��ֵԽ��Ȩ��Խ��
const Function_map_t stFunTab[] =
{
	{user_help, 			"?", 				0},
	{software_reset, 		"reboot", 			0},
	{tester_login, 			USER_NAME_TESTER,   1},
	{timer_list_print, 		"timerprint", 		0},
	{task_list_print,  		"taskprint", 		0},
	{timer_test, 		 	"timertest", 		1},
	{task_test, 			"tasktest", 		1},
	{task_list_pop, 		"pop", 				0},
	{admin_login, 			USER_NAME_ADMIN, 	1},
};
#define STFUNTAB_SIZE		(sizeof(stFunTab) / sizeof(stFunTab[0]))

//�ڲ��ն�ģ��ṹ�嶨��
static Terminal_t stTerminal;


/*********************************************************
* Name		: terminal_handler
* Function	: �ն���������������
* Input		: None
* Output	: None
* Return	: None
* Note		: �ú����ʺϷ��ں�̨�����е���
*********************************************************/
void terminal_handler(void)
{
	const char backSpace[3] = {'\b',' ','\b'};
	
	//����ͬ���뻥�������
	if (TERMINAL_READY != stTerminal.Flag.Bit.State)
	{
		input_echo();	//�������
		return;
	}
	
	switch (stTerminal.byCtrlType)
	{
		case '\b':	//backspace ����
		{
			stTerminal.byRecvLen = stTerminal.byRecvLen ? stTerminal.byRecvLen - 1 : 0;
			output_string(backSpace, 3);
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
			break;
		}

		case INPUT_KEY_UP:	//������
		{
			if (stTerminal.byRecvLen == 0)
			{
				if (' ' == stTerminal.byRecvBuff[stTerminal.byRecvLast - 1])
				{
					stTerminal.byRecvLast--;
				}
				
				stTerminal.byRecvLen = stTerminal.byRecvLast;
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLen);
			}
			break;
		}

		case INPUT_KEY_DOWN: //������
		{
			if (stTerminal.byRecvLen == 0)
			{
				stTerminal.byRecvLen = stTerminal.byRecvLast;
				output_string(stTerminal.byRecvBuff, stTerminal.byRecvLen);
				
				printf("\r\n");
				recv_semantic_analysis();
			}
			break;
		}

		case RECV_BUFF_OVERFLOW:	//���������
		{
			printf("uart recv over flow\r\n");
			stTerminal.byRecvLast = stTerminal.byRecvLen;
			stTerminal.byRecvLen = 0;
			break;
		}
		default :
			break;
	}
	stTerminal.Flag.Bit.State = TERMINAL_IDEL;
}

/*********************************************************
* Name		: terminal_init
* Function	: �ն�ģ�������ʼ��
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void terminal_init(void (*pCallBack)(uint8_t))
{
	uint8_t i;
	memset(&stTerminal, 0, sizeof(Terminal_t));
	stTerminal.OutPutCallBack = pCallBack; //���ڴ�ӡ����
	stTerminal.Flag.Bit.State = TERMINAL_IDEL;
	stTerminal.byAuthority = DEFAULT_AUTHORITY;
	
	//�����б���
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (NULL == stFunTab[i].func)
		{
			printf("stFunTab[%u] fun is NULL\r\n", i);
		}
		
		if (0 == strlen(stFunTab[i].pName))
		{
			printf("stFunTab[%u] Name is NULL\r\n", i);
		}
		
		if (FUN_ARGUMENTS_MAX_SIZE < stFunTab[i].byParamterNum)
		{
			printf("stFunTab[%u] ParamterNum is overflow\r\n", i);
		}
		
		//�����û���½������
		if (0 == strcmp(stFunTab[i].pName, USER_NAME_TESTER))
		{
			stTerminal.byUserIndexTab[0] = i;
		}
		else if (0 == strcmp(stFunTab[i].pName, USER_NAME_ADMIN))
		{
			stTerminal.byUserIndexTab[1] = i;
		}
	}
	
}


/*********************************************************
* Name		: input_echo
* Function	: ������Դ���
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void input_echo(void)
{
	if (stTerminal.byShowLen < stTerminal.byRecvLen)
	{
		stTerminal.OutPutCallBack(stTerminal.byRecvBuff[stTerminal.byShowLen++]);
	}
}

/*********************************************************
* Name		: output_string
* Function	: �����ַ���������
* Input		: const char *pStr	�ַ�����ʼ��ַ
			  uint8_t byLen		�ַ�������
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void output_string(const char *pStr, uint8_t byLen)
{
	ERROR("pointer is null", (pStr != NULL), return;);

	while (byLen--)
	{
		stTerminal.OutPutCallBack(*pStr++);
	}
	stTerminal.byShowLen = stTerminal.byRecvLen;
}

/*********************************************************
* Name		: check_login_cmd
* Function	: ����Ƿ��ǵ�½����
* Input		: uint8_t byIndex	��������
* Output	: None
* Return	: true �����û���½����		false �������û���½����
*********************************************************/
static bool check_login_cmd(uint8_t byIndex)
{
	uint8_t i = 0;
	for ( i = 0; i < USER_NUMBER; i++)
	{
		if (stTerminal.byUserIndexTab[i] == byIndex)
		{
			return true;
		}
	}
	return false;
}

/*********************************************************
* Name		: terminal_input_predeal
* Function	: �ն�����Ԥ����
* Input		: uint8_t byData	���ڶ�ȡ��������
* Output	: None
* Return	: None
* Note		: �ú������ڽ����ж��е���Ϊ��
*********************************************************/
void terminal_input_predeal(uint8_t byData)
{
	//ready״̬��ֱ���˳����ȴ�����������������
	if (stTerminal.Flag.Bit.State == TERMINAL_READY)
	{
		return;
	}
	//����ASCII���ַ��� ���账��
	if (byData > 127)
	{
		return;
	}

	//��������ַ����������ң�����
	if (stTerminal.bySpecialCharFlag)
	{
		stTerminal.bySpecialCharFlag++;

		switch (stTerminal.bySpecialCharFlag)
		{
			case 2:
			{
				//���������ַ��Ŀ�����
				if (91 == byData)
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
					stTerminal.Flag.Bit.State = TERMINAL_READY;
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
	if (byData < 32)
	{
		if (27 == byData)
		{
			stTerminal.bySpecialCharFlag = 1;
		}
		else	//����������ַ�
		{
			stTerminal.Flag.Bit.State = TERMINAL_READY;
			stTerminal.byCtrlType = byData;
		}
	}
	else	//����ASCII���ӡ�ַ�
	{
		//�������������
		//stTerminal.OutPutCallBack(byData);

		//���ջ���������ǿ���л����������״̬
		if (RECV_BUFF_MAX_SIZE == stTerminal.byRecvLen)
		{
			stTerminal.Flag.Bit.State = TERMINAL_READY;
			stTerminal.byCtrlType = RECV_BUFF_OVERFLOW;
		}
		else
		{
			stTerminal.byRecvBuff[stTerminal.byRecvLen++] = byData;
			stTerminal.Flag.Bit.State = TERMINAL_BUSY;
		}
	}
}

/*********************************************************
* Name		: searching_command
* Function	: �����Ѿ���������ݲ��ҷ��ϵ�ǰȨ�޵���������˵��û���½����
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void searching_command(void)
{
	uint8_t i = 0;
	uint8_t byNameLength;
	uint8_t byMatchIndex[STFUNTAB_SIZE] = {0};
	uint8_t byMatchNum = 0;
	uint8_t byNewLen = stTerminal.byRecvLen;
	bool bMatchFlag = true;
	char byChar;

	//���ջ�����Ϊ�գ����ش�ӡ��������
	if (0 == stTerminal.byRecvLen)
	{
		printf("\r\n");
		for (i = 0; i < stTerminal.byAuthority; i++)
		{
			if (false == check_login_cmd(i))
			{
				printf("%s   ", stFunTab[i].pName);
			}
		}
		printf("\r\n");
		return;
	}

	//��������ƥ��������¼������
	for (i = 0; i < stTerminal.byAuthority; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, stTerminal.byRecvBuff, stTerminal.byRecvLen))
		{	
			if (false == check_login_cmd(i))
			{
				byMatchIndex[byMatchNum++] = i;//�洢ƥ�����������
			}
		}
	}

	if (byMatchNum > 1)
	{
		printf("\r\n");
		for (i = 0; i < byMatchNum; i++)
		{
			printf("%s   ", stFunTab[byMatchIndex[i]].pName);
		}
		printf("\r\n");

		//������ͬ���ַ����������add1 add2������a֮�󣬰�tab��ʾadd
		while (bMatchFlag)
		{
			byChar = stFunTab[byMatchIndex[0]].pName[byNewLen];
			for (i = 1; i < byMatchNum; i++)
			{
				if (byChar != stFunTab[byMatchIndex[i]].pName[byNewLen])
				{
					bMatchFlag = false;
					break;
				}
			}
			byNewLen++;
		}
		
		byNewLen--;
		strncpy(&stTerminal.byRecvBuff[stTerminal.byRecvLen], &stFunTab[byMatchIndex[0]].pName[stTerminal.byRecvLen], byNewLen - stTerminal.byRecvLen);
		stTerminal.byRecvLen = byNewLen;
		output_string(stTerminal.byRecvBuff, stTerminal.byRecvLen);
	}
	else if (1 == byMatchNum)	//ƥ����������һ��
	{
		byNameLength = strlen(stFunTab[byMatchIndex[0]].pName);
		byNewLen = byNameLength - stTerminal.byRecvLen;		//�����ӵ��ַ�������
		strncpy(&stTerminal.byRecvBuff[stTerminal.byRecvLen], &stFunTab[byMatchIndex[0]].pName[stTerminal.byRecvLen], byNewLen);
		stTerminal.byRecvBuff[byNameLength++] = ' '; 		//��ӿո��
		
		stTerminal.byRecvLen = byNameLength;
		output_string(&stTerminal.byRecvBuff[stTerminal.byShowLen], byNewLen + 1);
	}
}

/*********************************************************
* Name		: recv_semantic_analysis
* Function	: ������������ݷ�������
* Input		: None
* Output	: None
* Return	: true���������	false���������
* Note		: None
*********************************************************/
static bool recv_semantic_analysis(void)
{
	uint8_t bState = true;
	uint8_t i, byCmdLen;
	uint8_t byTemp;
	uint8_t byHead = 0, byTail = 0;
	uint32_t dwPramter[FUN_ARGUMENTS_MAX_SIZE] = {0};	 //����������
	uint8_t byParamterNum = 0;			//��������
	uint8_t byCmdIndex = STFUNTAB_SIZE;	//ƥ������λ��
	if (0 == stTerminal.byRecvLen)	//���ݶ�Ϊ��
	{
		bState = false;
		goto SEMANTIC_ERROR;
	}
	
	//��ȡ����������ַ���
	byCmdLen = separate_string(&byHead, ' ', &byTail);
	if (0 == byCmdLen)
	{
		printf("input error\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}
	
	//��������
	for (i = 0; i < STFUNTAB_SIZE; i++)
	{
		if (0 == strncmp(stFunTab[i].pName, &stTerminal.byRecvBuff[byHead], byCmdLen))
		{
			byCmdIndex = i; //��¼�����λ��
			break;
		}
	}
	if (STFUNTAB_SIZE == byCmdIndex)	//δ�ҵ�����
	{
		printf("not support cmd\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}

	//��������Ĳ���
	for (i = byTail; i < stTerminal.byRecvLen; i += byTemp)
	{
		byHead = byTail + 1;
		byTemp = separate_string(&byHead, ' ', &byTail);
		if (0 == byTemp || byParamterNum >  stFunTab[byCmdIndex].byParamterNum)
		{
			break;
		}

		if (false == string_to_uint(&stTerminal.byRecvBuff[byHead], byTemp, &dwPramter[byParamterNum++]))
		{
			printf("paramater error\r\n");
			bState = false;
			goto SEMANTIC_ERROR;
		}
	}

	if (byParamterNum != stFunTab[byCmdIndex].byParamterNum)
	{
		printf("paramter num not match\r\n");
		bState = false;
		goto SEMANTIC_ERROR;
	}
	
	execute_handled(byCmdIndex, dwPramter, byParamterNum);
	
	SEMANTIC_ERROR:
	if (stTerminal.byRecvLen)	//�������µ����ݳ��ȣ���λ���ռ����Ի�����
	{
		stTerminal.byRecvLast = stTerminal.byRecvLen;
	}
	stTerminal.byRecvLen = 0;
	stTerminal.byShowLen = 0;
	
	return bState;
}


/*********************************************************
* Name		: separate_string
* Function	: ���ݷָ����Խ��յ����ݽ��зֶ�
* Input		: uint8_t *pStart �ָ���ʼλ��
			  const char chr �ָ���
* Output	: uint8_t *pEnd	�ַ����ָ�����λ��
* Return	: ƥ����ַ�������
* Note		: pEnd�Ľ���λ��Ϊ��һ���ָ���
*********************************************************/
static uint8_t separate_string(uint8_t *pStart, const char chr, uint8_t *pEnd)
{
	uint8_t byLen = 0;
	ERROR("pointer is null", (pStart != NULL && pEnd != NULL), return false;);
	//���˶���ķָ���
	while (*pStart < stTerminal.byRecvLen)
	{
		if (stTerminal.byRecvBuff[*pStart] != chr)
		{
			break;
		}
		++(*pStart);
	}

	//�����ַ���
	*pEnd = *pStart;
	while (*pEnd < stTerminal.byRecvLen)
	{
		if (stTerminal.byRecvBuff[*pEnd] == chr)
		{
			break;
		}
		++(*pEnd);
	}
	return *pEnd - *pStart;
}


/*********************************************************
* Name		: string_to_uint
* Function	: ���ַ���ת��Ϊ����
* Input		: char *pStr �ַ�����ʼ��ַ
			  uint8_t byLen	�ַ�������
* Output	: uint32_t *pValue ת�������ֵ
* Return	: true �ַ���ת���ɹ�  false ת������
* Note		: ��֧��16λ���ƺ�10����������
*********************************************************/
static bool string_to_uint(char *pStr, uint8_t byLen, uint32_t *pValue)
{
	uint32_t dwBaseValue = 10;
	uint32_t dwCharValue;
	bool bHexFlag = false;

	ERROR("pointer is null", (pStr != NULL && pValue != NULL), return false;);

	if ('0' == pStr[0] && ('x' == pStr[1] || 'X' == pStr[1]))
	{
		if (byLen > 10)	//�������
		{
			return false;
		}
		byLen -= 2;
		pStr += 2;
		dwBaseValue = 16;
		bHexFlag = true;
	}

	*pValue = 0;

	if (false == bHexFlag)
	{
		while (byLen--)
		{
			if (*pStr > '9' || *pStr < '0')
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
		while (byLen--)
		{
			//�������
			if (*pStr <= '9' && *pStr >= '0')
			{
				dwCharValue = *pStr - '0';
			}
			else if (*pStr <= 'f' && *pStr >= 'a')
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


/*********************************************************
* Name		: execute_handled
* Function	: ִ�н���������
* Input		: uint8_t byFunIndex 	��Ҫ���еĺ����ں����б��λ��
			  uint32_t *pArg	  	�����б�
			  uint8_t byArgNum		��������
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void execute_handled(uint8_t byFunIndex, uint32_t *pArg, uint8_t byArgNum)
{
	if (stTerminal.byAuthority <= byFunIndex)
	{	
		if (false == check_login_cmd(byFunIndex))
		{
			printf("Please switch user to improve authority!\r\n");
			return;
		}
	}
	switch (byArgNum)
	{
		case 0:
		{
			((void (*)())stFunTab[byFunIndex].func)();
			break;
		}
		case 1:
		{
			((void (*)(uint32_t))stFunTab[byFunIndex].func)(pArg[0]);
			break;
		}
		case 2:
		{
			((void (*)(uint32_t, uint32_t))stFunTab[byFunIndex].func)(pArg[0], pArg[1]);
			break;
		}
		case 3:
		{
			((void (*)(uint32_t, uint32_t, uint32_t))stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2]);
			break;
		}
		case 4:
		{
			((void (*)(uint32_t, uint32_t, uint32_t, uint32_t))stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2], pArg[3]);
			break;
		}
		case 5:
		{
			((void (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))stFunTab[byFunIndex].func)(pArg[0], pArg[1], pArg[2],
			pArg[3], pArg[4]);
			break;
		}
	}
}

/*********************************************************
* Name		: tester_login
* Function	: �û���½У�麯��
* Input		: uint32_t key 		�û���Կ
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void tester_login(uint32_t key)
{
	if (USER_KEY_TESETER == key)
	{
		stTerminal.byAuthority = stTerminal.byUserIndexTab[0];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*********************************************************
* Name		: admin_login
* Function	: �û���½У�麯��
* Input		: uint32_t key 		�û���Կ
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void admin_login(uint32_t key)
{
	if (USER_KEY_ADMIN == key)
	{
		stTerminal.byAuthority = stTerminal.byUserIndexTab[1];
		printf("UnLock success\r\n");
	}
	else
	{
		printf("UnLock fail\r\n");
	}
}

/*********************************************************
* Name		: software_reset
* Function	: �����λ
* Input		: None
* Output	: None
* Return	: None
* Note		: ����ʵ���뵥Ƭ��ƽ̨�������
*********************************************************/
static void software_reset(void)
{
	__set_FAULTMASK(1);   //STM32���������λ  
	NVIC_SystemReset();
}

/*********************************************************
* Name		: user_help
* Function	: �����λ
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
static void user_help(void)
{
	printf("\r\n***************** user manual ******************\r\n");
	printf("* %-12s  %-30s *\r\n", "Tab", "show and auto fill cmd");
	printf("* %-12s  %-30s *\r\n", "Up", "show last input");
	printf("* %-12s  %-30s *\r\n", "Down", "execute last input");
	printf("* %-12s  %-30s *\r\n", "Enter", "execute cmd");
	printf("* %-12s  %-30s *\r\n", "Backspace", "delete character");
	printf("************************************************\r\n");
}