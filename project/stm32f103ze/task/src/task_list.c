/********************************************************
*file name: task_list.c
*function: ʵ������Ĺ����ṩ��ӡ�ɾ����ʱ���Ľӿ�
*ʵ�֣������������ķ�ʽʵ�֣���ͷ��ָ�룬���Ǵ���β�������
	   ������һ�����������������ȼ������ȼ��ߵ����ײ���
	   �������ȼ���ͬ������ӵ�������ִ��
	   ����ID 0~31 ��������������Ψһ���ڵģ�������������ͬ��ID����
	   �����ͻ���������Ѿ��������ID��ͻ�����µ���ͬ���������
	   ��ͬ��������Թ��������ID��Ϊ 32~254
	   ����ID�����ֵΪ0xFF����������ռ�ã���ID��ЧֵΪ0~0xFE
	   �������ȼ�ֵΪ1~0xFF����ֵ������ȼ��ߣ����ȼ�0�ı�����ռ��
	   
*note: ���������ɾ������Ӳ��ʺ����ж���ִ�У����б�Ҫ���������������ת
********************************************************/
#include <string.h>
#include "task_list.h"
#include "debug.h"
#include "task_manager.h"

//ģ����˽�б�������
static volatile Task_list_t stTask;

//ģ����˽�к�������
static bool task_list_add(Task_node_t *pTask);
static void task_list_priority_increase(void);

/*********************************************************/
//name: task_list_init
//function: ���������ʼ��
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
void task_list_init(void)
{
	memset((uint8_t *)&stTask, 0, sizeof(stTask));
	
	//����β�����
	stTask.List[TASK_GUARTD_INDEX].byID = TASK_GUARTD_ID;
	stTask.List[TASK_GUARTD_INDEX].byPriority = TASK_GUARTD_PRIORITY; //������ȼ�
	stTask.List[TASK_GUARTD_INDEX].CallBack = NULL;
	stTask.List[TASK_GUARTD_INDEX].byNext = TASK_GUARTD_INDEX;
	
	stTask.byHead = TASK_GUARTD_INDEX;
}

#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)
/*********************************************************/
//name: task_list_priority_increase
//function: �������ȼ���ʱ��������
//input: None
//output: None
//return: None
//note: None
/*********************************************************/
static void task_list_priority_increase(void)
{
	uint8_t i;
	uint8_t byHead = stTask.byHead;
	for(i = 0; i < stTask.byNum; i++)
	{
		stTask.List[byHead].byPriority++;
		byHead = stTask.List[byHead].byNext;
	}
}
#endif

/*********************************************************/
//name: task_list_pop
//function: ��������������ִ��������
//input: None
//output: None
//return: true ����ִ�����    false ����Ϊ��
//note: None
/*********************************************************/
bool task_list_pop(void)
{
	if(stTask.byNum)
	{
		printf("%u", stTask.List[stTask.byHead].byID);
		stTask.List[stTask.byHead].CallBack();
		
		stTask.bMemFlag[stTask.byHead] = false;
		if(stTask.List[stTask.byHead].byID < TASK_MAX_UNIQUE_ID)
		{
			stTask.bIDFlag[stTask.List[stTask.byHead].byID] = false;
		}
		
		stTask.byHead = stTask.List[stTask.byHead].byNext;
		stTask.byNum--;
#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)		
		task_list_priority_increase();
#endif		
		return true;
	}
	return false;
}

/*********************************************************/
//name: task_list_add
//function: �������������������
//input: Task_node_t *pTask ����ṹ��ָ��
//output: None
//return: true ��ӳɹ�		false ���ʧ��
//note: None
/*********************************************************/
static bool task_list_add(Task_node_t *pTask)
{
	uint8_t index = stTask.byMemIndex;
	
	volatile uint8_t *pPre = &stTask.byHead;
	uint8_t byHead = stTask.byHead;
	uint8_t byNext = stTask.byHead;
	
	TASK_ERROR("system pTask memeroy is full", TASK_MAX_SIZE > stTask.byNum, return false);
	TASK_ERROR("pointer is null", NULL != pTask, return false);
	
	//���ҿ��е��ڴ��
	while(true == stTask.bMemFlag[index])
	{
		index = (index + 1) % TASK_MAX_SIZE;
		
	}
	//���������ڴ��
	memcpy((uint8_t *)&stTask.List[index], pTask, sizeof(Task_node_t));
	
	//�������ȼ����Ҷ�Ӧ�Ĳ���λ��
	while(stTask.List[byNext].byPriority >= pTask->byPriority)
	{
		byHead = byNext;
		byNext = stTask.List[byNext].byNext;
	}
	//����λ�ò����ײ�
	if(stTask.byHead != byNext)
	{
		pPre = &stTask.List[byHead].byNext;
	}
	
	//�����������������
	*pPre = index;
	stTask.List[index].byNext = byNext;
	
	//��������״̬��
	stTask.byNum++;
	stTask.bMemFlag[index] = true;
	if(pTask->byID < TASK_MAX_UNIQUE_ID)
	{
		stTask.bIDFlag[pTask->byID] = true;
	}
	stTask.byMemIndex = (index + 1) % TASK_MAX_SIZE;  //�����ڴ����
	return true;
}

/*********************************************************/
//name: task_list_push
//function: �������������������  �������Ѿ����ڣ�����Դ˴��������
//input:
//output:
//return: true ��ӳɹ� 	false ���ʧ��
//note:
/*********************************************************/
bool task_list_push(Task_node_t *pTask)
{
	TASK_ERROR("pointer is null", NULL != pTask, return false);
	
	if(0 == pTask->byPriority)
	{
		TASK_PRINT(("task_priority_is_zero\r\n"));
		return false;
	}
	
	if (pTask->byID < TASK_MAX_UNIQUE_ID)
	{
		if(true == stTask.bIDFlag[pTask->byID])
		{
			TASK_PRINT(("task_id:%u already exists\r\n", pTask->byID));
			return false;
		}
	}
	
	return task_list_add(pTask);
}

/*********************************************************/
//name: task_list_delete
//function: ɾ������IDΪ byDeletID������
//input: uint8_t byDeletID ����ID��
//output: None
//return: None
//note: 
/*********************************************************/
void task_list_delete(uint8_t byDeletID)
{
	volatile uint8_t *pre = &stTask.byHead;
	uint8_t byNext = stTask.byHead;
	uint8_t flag;
	uint8_t byTempID;
	
	while (stTask.List[byNext].byID != TASK_GUARTD_ID)
	{
		//β���������
		stTask.List[TASK_GUARTD_INDEX].byID = byDeletID;

		while (stTask.List[byNext].byID != byDeletID)
		{
			pre = &stTask.List[byNext].byNext;
			byNext = stTask.List[byNext].byNext;
		}
		//�Ǳ����ɾ������
		if (byNext != TASK_GUARTD_INDEX)
		{
			//���ID��־λ
			if(byDeletID < TASK_MAX_UNIQUE_ID)
			{
				stTask.bIDFlag[byDeletID] = false;				
			}

			//ɾ������
			*pre = stTask.List[byNext].byNext;
			stTask.bMemFlag[byNext] = false;
			stTask.byNum--;
			byNext = stTask.List[byNext].byNext;
		}
		//�ָ��������
		stTask.List[TASK_GUARTD_INDEX].byID = TASK_GUARTD_ID;
	}
}
//----------------------------�������֧��--------------------------------------------------
#if (1 == TASK_DEBUG_ON)
/*********************************************************/
//name: task_list_print
//function: ��ӡ��������
//input:
//output:
//return:
//note:
/*********************************************************/
void task_list_print(void)
{
	uint8_t byNext = stTask.byHead;
	TASK_PRINT(("\r\n---- pTask list num %u ---\r\n", stTask.byNum));
	TASK_PRINT(("ID   Priority   FUN_ADDR\r\n"));
	
	while (TASK_GUARTD_INDEX != byNext)
	{
		TASK_PRINT(("%-5u", stTask.List[byNext].byID));
		TASK_PRINT(("%-11u", stTask.List[byNext].byPriority));
		TASK_PRINT(("0x%08x\r\n", (uint32_t)stTask.List[byNext].CallBack));
		byNext = stTask.List[byNext].byNext;
	}
}

#else
void task_list_print(void) {}

#endif










