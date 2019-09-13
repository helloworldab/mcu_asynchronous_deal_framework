#ifndef TIMER_TASK_H
#define TIMER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

//���֧�ֵĶ�ʱ����������
#define TIMER_MAX_SIZE		TIMER_MAX_NUM
#define TIMER_MEMORY_SIZE	(TIMER_MAX_SIZE + 1)


//����repeat��ֵΪ0XFF����־Ϊһֱ���ڵ�����
#define TIMER_CONITNUOUS_PERIOD 0xFF
#define TIMER_CONTINUOUS_VAL	(0xff - 1)

#define MAX_PERIOD_VAL		0xFFFF
#define MAX_TIMER_ID		0xFF
#define END_GUARD_INDEX		TIMER_MAX_NUM
typedef struct
{
	FUNC call_back;			//��ʱ���ص�
	uint16_t tick;			//��ʱ��ֵ
	uint16_t period;		//��������
	uint8_t repeat;			//�ظ�����
	uint8_t task_id;		//�����־��
	uint8_t next;			//��һ����ʱ������λ��
	//uint8_t res[1];
}Timer_node_t;

typedef struct
{
	Timer_node_t list[TIMER_MEMORY_SIZE];	//��ʱ�������
	bool mem_flag[TIMER_MAX_SIZE];			//�����¼����ڴ��п��б�־λ
	bool id_flag[TIMER_MAX_SIZE];			//Ψһ��ʱ��
	uint8_t mem_index;						//�ڴ��¼���ţ�����ǰ���ڴ���У�����һ���ڴ�����Ҳ�ǿ���
	uint8_t size;							//�����¼������¼��ĸ���
	uint8_t head;							//�����¼�����Ԫ��
}Timer_list_t;



void timer_list_init(void);
void timer_list_scan(void);
void timer_list_push(Timer_node_t *pNode);
bool timer_list_empty(void);
#endif