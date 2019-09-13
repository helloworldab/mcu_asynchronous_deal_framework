#include <string.h>
#include "task_list.h"
#include "debug.h"

static Task_list_t stTask;

/*********************************************************/
//name: sys_task_init
//function: ϵͳ���������ʼ��
//input:
//output:
//return: 
//note:
/*********************************************************/
void task_list_init(void)
{
	memset((uint8_t *)&stTask, 0, sizeof(stTask));
	
	//����β�����
	stTask.list[TASK_MAX_SIZE].priority = 0; //������ȼ�
	stTask.list[TASK_MAX_SIZE].call_back = NULL;
	stTask.head = TASK_MAX_SIZE;
}

#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)
/*********************************************************/
//name: sys_task_priority_increase
//function: ���������������������ȼ�
//input:
//output:
//return:
//note:
/*********************************************************/
void task_list_priority_increase(void)
{
	uint8_t i;
	uint8_t head = stTask.head;
	for(i = 0; i < stTask.size; i++)
	{
		stTask.list[head].priority++;
		head = stTask.list[head].next;
	}
}
#endif

/*********************************************************/
//name: task_list_pop
//function: ϵͳ���������
//input:
//output:
//return:
//note:
/*********************************************************/
FUNC task_list_pop(void)
{
	FUNC fun = NULL;
	if(stTask.size)
	{
		fun = stTask.list[stTask.head].call_back;
		stTask.mem_flag[stTask.head] = false;
		stTask.head = stTask.list[stTask.head].next;
		stTask.size--;
#if (1 == USE_PRIORITY_INCREASE_WITH_TIME)		
		task_list_priority_increase();
#endif
	}
	return fun;
}

/*********************************************************/
//name: task_list_push
//function: �������������������
//input:
//output:
//return:
//note:
/*********************************************************/
void task_list_push(Task_node_t *ptask)
{
	uint8_t index = stTask.mem_index;
	
	uint8_t *pPre = &stTask.head;
	uint8_t head = stTask.head;
	uint8_t next = stTask.head;
	
	TASK_ASSERT("pointer is null", NULL != ptask);
	
	TASK_ERROR("system task memeroy is full", TASK_MAX_SIZE > stTask.size, return);
	TASK_ERROR("pointer is null", NULL != ptask, return);
	
	//���ҿ��е��ڴ��
	while(true == stTask.mem_flag[index])
	{
		index = (index + 1) % TASK_MAX_SIZE;
	}
	
	//�������ȼ����Ҷ�Ӧ�Ĳ���λ��
	if(stTask.size)
	{
		while(stTask.list[next].priority >= ptask->priority)
		{
			head = next;
			next = stTask.list[next].next;
		}
		
		//����λ�ò����ײ�
		if(stTask.head != next)
		{
			pPre = &stTask.list[head].next;
		}
	}
	
	//�����������������
	*pPre = head;
	stTask.list[index].next = next;
	stTask.list[index].call_back = ptask->call_back;
	stTask.list[index].priority = ptask->priority;
	
	//��������״̬��
	stTask.size++;
	stTask.mem_flag[index] = true;
	stTask.mem_index = (index + 1) % TASK_MAX_SIZE;  //�����ڴ����
}

/*********************************************************/
//name: sys_task_print
//function: �������������������
//input:
//output:
//return:
//note:
/*********************************************************/
void task_list_print(void)
{
	uint8_t i;
	uint8_t head;
	printf("stTask.size %u\r\n", stTask.size);
	
	printf("memory flag:");
	for(i = 0; i < TASK_MAX_SIZE; i++)
	{
		printf("%01x ", stTask.mem_flag[i]);
	}
	printf("\r\n");
	
	for(i = 0, head = stTask.head; i < stTask.size; i++)
	{
		printf("[index %u, pro %u]->", head, stTask.list[head].priority);
		head = stTask.list[head].next;
	}
	printf("[NULL]\r\n");
}










