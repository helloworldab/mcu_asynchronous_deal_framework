/********************************************************
*file name: timer_list.c
*function: ʵ�ֶ�ʱ������Ĺ����ṩ��ӡ�ɾ������Ľӿ�
*ʵ�֣������������ķ�ʽʵ�֣���ͷ��ָ�룬���Ǵ���β�������
	   ������һ��������������ʱ�䣬����ִ�е����ײ���
	   ��ĳһʱ�̣�������ʱ������ͬʱʱ�䵽��������ִ������ӵ�����
	   ��ʱ����߷ֱ���Ϊ1ms����ͷֱ���Ϊtimer_list_scanִ�е�Ƶ��
	   ��ʱ�������ִ���ǣ����ӳ٣���ִ��

*note: timer_list_scan()������Ҫ�����ں�̨��������Ҫ���еĶ�ʱ������
********************************************************/
#include <string.h>
#include "timer_list.h"
#include "intermediate_time.h"
#include "debug.h"

static void timer_list_add(Timer_node_t *pNode, uint8_t mem_index);
void timer_list_print(void);

static Timer_list_t stTimer;

/*********************************************************/
//name: timer_task_init
//function: ��ʱ�����������ʼ��
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_init(void)
{
	memset((uint8_t *)&stTimer, 0, sizeof(stTimer));

	//����β�����
	stTimer.list[END_GUARD_INDEX].task_id = MAX_TIMER_ID;
	stTimer.list[END_GUARD_INDEX].tick = 0;
	stTimer.list[END_GUARD_INDEX].period = MAX_PERIOD_VAL;
	stTimer.list[END_GUARD_INDEX].call_back = NULL;
	stTimer.head = END_GUARD_INDEX;
}

/*********************************************************/
//name: timer_list_scan
//function: ��ʱ����������ɨ�躯������mian����while(1)�е���
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_scan(void)
{
	FUNC fun = NULL;
	uint8_t head = stTimer.head;
	uint8_t next;
	uint8_t period;

	uint8_t exec_num = 0;
	uint8_t exec_index[TIMER_MAX_SIZE] = {0};
	uint8_t exec_id;
	uint8_t i;

	uint16_t tick = get_timer_tick();
	if (0 == stTimer.size)
		return;
	//��β���
	stTimer.list[TIMER_MAX_SIZE].tick = tick;
	while ((uint16_t)(tick - stTimer.list[head].tick) > stTimer.list[head].period)
	{
		if (NULL != stTimer.list[head].call_back)
		{
			printf("%02u", stTimer.list[head].task_id);
			stTimer.list[head].call_back();
		}

		if (--stTimer.list[head].repeat)
		{
			exec_index[exec_num++] = head;//�ݴ滺����
		}
		else
		{
			exec_id = stTimer.list[head].task_id < TIMER_MAX_SIZE ? stTimer.list[head].task_id : TIMER_MAX_SIZE;
			stTimer.id_flag[exec_id] = false;
			stTimer.mem_flag[exec_index[i]] = false; //����ڴ����
		}
		stTimer.size--;
		next = stTimer.list[head].next;//��һ��ѭ��ɨ���ж�
		head = next;
	}
	stTimer.head = head;

	for (i = 0; i < exec_num; i++)
	{
		//����ʱ��������һֱ���ڵģ���ָ�repeat��ֵΪ0xFF
		stTimer.list[exec_index[i]].repeat += (TIMER_CONTINUOUS_VAL == stTimer.list[exec_index[i]].repeat);
		timer_list_add(NULL, exec_index[i]);
		timer_list_print();
	}
}

/*********************************************************/
//name: timer_list_push
//function: ����ʱ�����������������
//input:
//output:
//return:
//note:�����������ڽ�����ĳ��ʱ�̣�ͬʱ��ʱ�䣬������ӵ���������ִ��
/*********************************************************/
static void timer_list_add(Timer_node_t *pNode, uint8_t mem_index)
{
	uint8_t index = stTimer.mem_index;

	uint8_t *pPre = &stTimer.head;
	uint8_t head = stTimer.head;
	uint8_t next = stTimer.head;

	TASK_ERROR("timer task memeroy is full", (TIMER_MAX_SIZE > stTimer.size), return;);

	if (stTimer.size >=  TIMER_MAX_SIZE)
		return;

	uint16_t tick = get_timer_tick();//��õ�ǰ�Ķ�ʱ����ֵ
	uint16_t dia_tick;

	//�ڴ���в����ڸö�ʱ������,��Ҫ�����ڴ�
	if (TIMER_MAX_SIZE == mem_index)
	{
		//���ҿ��е��ڴ��
		while (true == stTimer.mem_flag[index])
		{
			index = (index + 1) % TIMER_MAX_SIZE;
		}
		//���ƶ�ʱ�������ڴ��
		memcpy(&stTimer.list[index], pNode, sizeof(Timer_node_t));
		mem_index = index;

		//�����ڴ����
		stTimer.mem_flag[index] = true;
		stTimer.mem_index = (index + 1) % TIMER_MAX_SIZE;
	}
	//���浱ǰ��tickֵ
	stTimer.list[mem_index].tick = tick;

	if (stTimer.size)
	{
		//β�����
		stTimer.list[TIMER_MAX_SIZE].tick = tick;

		dia_tick = tick - stTimer.list[next].tick;
		//���˵�ʱ���ѵ������ǻ�δ���ü����������
		while (stTimer.list[next].period <= dia_tick)
		{
			head = next;
			next = stTimer.list[next].next;
			dia_tick = tick - stTimer.list[next].tick;
		}
		//���Ҳ���λ��
		while (stTimer.list[mem_index].period >= (uint16_t)(stTimer.list[next].period - dia_tick))
		{
			head = next;
			next = stTimer.list[next].next;
			dia_tick = tick - stTimer.list[next].tick;
		}

		//����λ�ò���ͷ��
		if (stTimer.head != next)
		{
			pPre = &stTimer.list[head].next;
		}
	}

	*pPre = mem_index;
	stTimer.list[mem_index].next = next;
	stTimer.size++;
}

/*********************************************************/
//name: timer_list_push
//function: ��ʱ�����������������
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_push(Timer_node_t *pNode)
{
	TASK_ASSERT("pointer is null", NULL != pNode);
	TASK_ERROR("pointer is null", NULL != pNode, return;);
	timer_list_add(pNode, TIMER_MAX_SIZE);
}

/*********************************************************/
//name: timer_list_delete
//function: ɾ������IDΪ task_ID�Ķ�ʱ������
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_delete(uint8_t task_ID)
{
	uint8_t head = stTimer.head;
	uint8_t next = head;
	uint8_t flag;
	while (stTimer.list[next].task_id != MAX_TIMER_ID)
	{
		//β���������
		stTimer.list[END_GUARD_INDEX].task_id = task_ID;

		while (stTimer.list[next].task_id != task_ID)
		{
			head = next;
			next = stTimer.list[next].next;
		}
		//�Ǳ����ɾ������
		if (next != MAX_TIMER_ID)
		{
			stTimer.list[head].next = stTimer.list[next].next;
			stTimer.mem_flag[next] = false;
			stTimer.size--;

			next = stTimer.list[next].next;
		}
		//�ָ��������
		stTimer.list[END_GUARD_INDEX].task_id = MAX_TIMER_ID;
	}
}
/*********************************************************/
//name: timer_list_push
//function: ��ʱ�����������������
//input:
//output:
//return:
//note:
/*********************************************************/
bool timer_list_empty(void)
{
	return !stTimer.size;
}


//----------------------------��ʱ���������֧��--------------------------------------------------
#if (1 == TIMER_TASK_DEBUG_ON)
/*********************************************************/
//name: timer_list_print
//function: ��ʱ�����������������
//input:
//output:
//return:
//note:
/*********************************************************/
void timer_list_print(void)
{
	uint8_t next = stTimer.head;
	printf("\r\n----timer task list size %u ---\r\n", stTimer.size);
	while (stTimer.list[next].period != MAX_PERIOD_VAL)
	{
		printf("ID:%2u ", stTimer.list[next].task_id);
		printf("repeat:%2u ", stTimer.list[next].repeat);
		printf("period:%2u ", stTimer.list[next].period);
		printf("tick:%2u \r\n", stTimer.list[next].tick);
		next = stTimer.list[next].next;
	}
}

#else
void timer_list_print(void) {}

#endif
