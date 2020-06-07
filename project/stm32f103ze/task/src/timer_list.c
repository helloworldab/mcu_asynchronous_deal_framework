/********************************************************
*file name: timer_list.c
*function: ʵ�������ʱ������Ĺ����ṩ��ӡ�ɾ����ʱ���Ľӿ�
*ʵ�֣������������ķ�ʽʵ�֣���ͷ��ָ�룬���Ǵ���β�������
	   ������һ��������������ʱ�䣬����ִ�е����ײ���
	   ��ĳһʱ�̣�������ʱ������ͬʱʱ�䵽��������ִ������ӵ�����
	   ��ʱ����߷ֱ���Ϊ1ms����ͷֱ���Ϊtimer_list_popִ�е�Ƶ��
	   ��ʱ����ִ���ǣ����ӳ٣���ִ��
	   ��ʱ��ID 0~31 ��������������Ψһ���ڵģ�������������ͬ��ID����
	   ��ʱ����ͻ������ʱ���Ѿ��������ID��ͻ�������ͬ������ɾ���������
	   ��ʱ��ID ���ֵΪ0xFF����������ռ�ã���ID��ЧֵΪ0~0xFE

*note: timer_list_pop()
						1.��ǰ̨����
							����Ҫ�����̰߳�ȫ���⣬��ʱ��������ģ��С��������
							���Ƕ�ʱ��������Ӧʱ��ò�����֤����Ӧʱ����ϵͳ��ģӰ��ϴ�
						2.�ں�̨���У����ô˷�����
							��ʱ���������ж���ִ�У�������ģ���˹���
							��Ҫ�����̰߳�ȫ���⣬������Ը���
							��ʱ����Ӧʱ��õ���֤����ϵͳ��ģӰ��С
		timer_list_pop()�����������ж��е��ã������ĺ�����Ӧ�����ж��е��ã�
		�����Ҫ�������Ȱ����������������е��У�Ȼ���ں�̨�����а����������������
		�����ж��е��ã���Ҫ�����̰߳�ȫ����
********************************************************/
#include <string.h>
#include "timer_list.h"
#include "intermediate_time.h"
#include "task_manager.h"
#include "debug.h"


//ģ����˽�б�������
static volatile Timer_list_t stTimer;

//ģ����˽�к�������
static void timer_list_add(Timer_node_t *pNode, uint8_t byMemIndex);

/*********************************************************
* Name		: timer_task_init
* Function	: ��ʱ�����������ʼ��
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void timer_list_init(void)
{
	memset((uint8_t *)&stTimer, 0, sizeof(stTimer));

	//����β�����
	stTimer.List[TIMER_GUARTD_INDEX].byID = TIMER_GUARTD_ID;
	stTimer.List[TIMER_GUARTD_INDEX].wTick = 0;
	stTimer.List[TIMER_GUARTD_INDEX].wPeriod = TIMER_GUARTD_PERIOD;
	stTimer.List[TIMER_GUARTD_INDEX].byRepeat = TIMER_GUARTD_REPEAT;
	stTimer.List[TIMER_GUARTD_INDEX].CallBack = NULL;	
	stTimer.byHead = TIMER_GUARTD_INDEX;
}

/*********************************************************
* Name		: timer_list_pop
* Function	: �����ʱ������ɨ�躯�����ڶ�ʱ���жϺ����е���
* Input		: None
* Output	: None
* Return	: None
* Note		: �������ж���ִ�У���ʱ����������ģӦ�����ܼ��
*********************************************************/
void timer_list_pop(void)
{
	uint8_t byHead = stTimer.byHead;

	uint8_t byExecNum = 0;
	uint8_t byExecIndex[TIMER_MAX_SIZE] = {0};
	uint8_t i;

	uint16_t wTick;

	if (0 == stTimer.byNum)
	{
		return;
	}
	//�жϴ���˶�ʱ���������˳����������ã���֤��ʱ���������ݰ�ȫ
	if (stTimer.byMutex) 
	{
		return;
	}

	//��β���
	wTick = get_timer_tick();
	stTimer.List[TIMER_MAX_SIZE].wTick = wTick;
	while ((uint16_t)(wTick - stTimer.List[byHead].wTick) > stTimer.List[byHead].wPeriod)
	{
		if (NULL != stTimer.List[byHead].CallBack)
		{
			printf("%-2u", stTimer.List[byHead].byID);
			stTimer.List[byHead].CallBack();
		}

		if (--stTimer.List[byHead].byRepeat)
		{
			byExecIndex[byExecNum++] = byHead;//�ݴ滺����
		}
		else
		{
			if (stTimer.List[byHead].byID < TIMER_MAX_UNIQUE_ID)
			{
				stTimer.bIDFlag[stTimer.List[byHead].byID] = false;
			}
			stTimer.bMemFlag[byHead] = false; //����ڴ����
		}
		stTimer.byNum--;
		byHead = stTimer.List[byHead].byNext;//��һ��ѭ��ɨ���ж�
	}
	stTimer.byHead = byHead;

	for (i = 0; i < byExecNum; i++)
	{
		//����ʱ��������һֱ���ڵģ���ָ�byRepeat��ֵΪ0xFF
		stTimer.List[byExecIndex[i]].byRepeat += (TIMER_CONTINUOUS_VAL == stTimer.List[byExecIndex[i]].byRepeat);
		timer_list_add(NULL, byExecIndex[i]);
		timer_list_print();
	}
}

/*********************************************************
* Name		: timer_list_add
* Function	: �������ʱ�������������
* Input		: Timer_node_t *pNode	��ʱ������ָ��
			  uint8_t byMemIndex		���ڴ��е�λ��
* Output	: None
* Return	: None
* Note		: ��������ʱ�������ڽ�����ĳ��ʱ�̣�ͬʱ��ʱ�䣬������ӵ���������ִ��
*********************************************************/
static void timer_list_add(Timer_node_t *pNode, uint8_t byMemIndex)
{
	uint8_t index = stTimer.byMemIndex;
	volatile uint8_t *pPre = &stTimer.byHead;
	uint8_t byHead = stTimer.byHead;
	uint8_t byNext = stTimer.byHead;

	ERROR("timer pTask memeroy is full", (TIMER_MAX_SIZE > stTimer.byNum), return;);

	if (stTimer.byNum >=  TIMER_MAX_SIZE)
		return;

	uint16_t wTick = get_timer_tick();//��õ�ǰ�Ķ�ʱ����ֵ
	uint16_t dia_tick;

	//�ڴ���в����ڸö�ʱ������,��Ҫ�����ڴ�
	if (TIMER_GUARTD_INDEX == byMemIndex)
	{
		//���ҿ��е��ڴ��
		while (true == stTimer.bMemFlag[index])
		{
			index = (index + 1) % TIMER_MAX_SIZE;
		}
		//���ƶ�ʱ�������ڴ��
		memcpy((uint8_t *)&stTimer.List[index], pNode, sizeof(Timer_node_t));
		byMemIndex = index;

		//�����ڴ����
		stTimer.bMemFlag[index] = true;
		stTimer.byMemIndex = (index + 1) % TIMER_MAX_SIZE;
		if(pNode->byID < TIMER_MAX_UNIQUE_ID)
		{
			stTimer.bIDFlag[pNode->byID] = true;
		}
	}	
	stTimer.List[byMemIndex].wTick = wTick;	//���浱ǰ��wTickֵ
	
	
	stTimer.List[TIMER_MAX_SIZE].wTick = wTick;		//����β�����ʱ��
	dia_tick = wTick - stTimer.List[byNext].wTick;	//���������ײ�ʱ���
	//���˵�ʱ���ѵ������ǻ�δ���ü����������
	while (stTimer.List[byNext].wPeriod <= dia_tick)
	{
		byHead = byNext;
		byNext = stTimer.List[byNext].byNext;
		dia_tick = wTick - stTimer.List[byNext].wTick;
	}
	//���Ҳ���λ��
	while (stTimer.List[byMemIndex].wPeriod >= (uint16_t)(stTimer.List[byNext].wPeriod - dia_tick))
	{
		byHead = byNext;
		byNext = stTimer.List[byNext].byNext;
		dia_tick = wTick - stTimer.List[byNext].wTick;
	}
	//����λ�ò���ͷ��
	if (stTimer.byHead != byNext)
	{
		pPre = &stTimer.List[byHead].byNext;
	}

	*pPre = byMemIndex;
	stTimer.List[byMemIndex].byNext = byNext;
	stTimer.byNum++;
}

/*********************************************************
* Name		: timer_list_push
* Function	: ��ʱ������������������������Ѿ����ڣ���ɾ���������
* Input		: Timer_node_t *pNode ������Ϣָ��
* Output	: None
* Return	: None
* Note		: �ú����������ж�������ã������Ҫ���ж�������Ӷ�ʱ������
			  ����ͨ���Ȱ����������������������У��ں�̨������ȡ���������������
*********************************************************/
void timer_list_push(Timer_node_t *pNode)
{
	TASK_ASSERT("pointer is null", NULL != pNode);
	ERROR("pointer is null", NULL != pNode, return;);
		
	task_enter_critical(); 	//�ٽ�α���������
	stTimer.byMutex++;		//��timer_list_pop�����γɻ������stTimer��Դ
	task_exit_critical();
	
	if (pNode->byID < TIMER_MAX_UNIQUE_ID)
	{
		if(true == stTimer.bIDFlag[pNode->byID])
		{
			timer_list_delete(pNode->byID);
			TIMER_PRINT(("timer_id:%u already exists\r\n", pNode->byID));
		}
	}

	timer_list_add(pNode, TIMER_GUARTD_INDEX);
	
	stTimer.byMutex--; //�����жϺ���ִ�ж�ʱ��ɨ�躯��
}

/*********************************************************
* Name		: timer_list_delete
* Function	: ɾ������IDΪ byID�Ķ�ʱ������
* Input		: uint8_t byDeletID ��ʱ������ID��
* Output	: None
* Return	: None
* Note		: �ú������������ж��е��ã�ԭ�򣺱����ٽ���Դ��stTimer��
			  ���������������������У���ת�������������У���ִ��
*********************************************************/
void timer_list_delete(uint8_t byDeletID)
{
	volatile uint8_t *pre = &stTimer.byHead;
	uint8_t byNext = stTimer.byHead;
	uint8_t flag;
	uint8_t byTempID;
	
	task_enter_critical(); 	//�ٽ�α��������� 
	stTimer.byMutex++;		//��timer_list_pop�����γɻ������stTimer��Դ
	task_exit_critical();
	
	while (stTimer.List[byNext].byID != TIMER_GUARTD_ID)
	{
		//β���������
		stTimer.List[TIMER_GUARTD_INDEX].byID = byDeletID;

		while (stTimer.List[byNext].byID != byDeletID)
		{
			pre = &stTimer.List[byNext].byNext;
			byNext = stTimer.List[byNext].byNext;
		}
		//�Ǳ����ɾ������
		if (byNext != TIMER_GUARTD_INDEX)
		{
			//���ID��־λ
			if(byDeletID < TIMER_MAX_UNIQUE_ID)
			{
				stTimer.bIDFlag[byDeletID] = false;				
			}

			//ɾ����ʱ������
			*pre = stTimer.List[byNext].byNext;
			stTimer.bMemFlag[byNext] = false;
			stTimer.byNum--;

			byNext = stTimer.List[byNext].byNext;
		}
		//�ָ��������
		stTimer.List[TIMER_GUARTD_INDEX].byID = TIMER_GUARTD_ID;
	}
	
	stTimer.byMutex--; //�����ź����ָ�
}

/*********************************************************
* Name		: timer_list_empty
* Function	: ��ѯ��ʱ�������Ƿ�Ϊ��
* Input		: None
* Output	: None
* Return	: true �����		false ����ǿ�
* Note		: None
*********************************************************/
bool timer_list_empty(void)
{
	return !stTimer.byNum;
}


//----------------------------��ʱ���������֧��--------------------------------------------------
#if (1 == TIMER_DEBUG_ON)
/*********************************************************
* Name		: timer_list_print
* Function	: ��ӡ��ʱ������
* Input		: None
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void timer_list_print(void)
{
	uint8_t byNext = stTimer.byHead;
	TIMER_PRINT(("\r\n----timer pTask list num %u ---\r\n", stTimer.byNum));
	TIMER_PRINT(("ID   Repeat   Period   FUN_ADDR   Tick\r\n"));
	
	while (TIMER_GUARTD_INDEX != byNext)
	{
		TIMER_PRINT(("%-5u", stTimer.List[byNext].byID));
		TIMER_PRINT(("%-9u", stTimer.List[byNext].byRepeat));
		TIMER_PRINT(("%-9u", stTimer.List[byNext].wPeriod));
		TIMER_PRINT(("0x%08x ", (uint32_t)stTimer.List[byNext].CallBack));
		TIMER_PRINT(("%4u\r\n", stTimer.List[byNext].wTick));
		byNext = stTimer.List[byNext].byNext;
	}
}

#else

void timer_list_print(void) {}

#endif
