#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "task_config.h"

#define TASK_QUEUE_MAX_SIZE		TASK_QUEUE_SIZE
#define TASK_TYPE_TIMER			1
#define TASK_TYPE_TASK			2


typedef struct
{
	union
	{
		struct
		{
			uint8_t byRepeat;		//��ʱ���ظ�����
			uint8_t byPriority;		//�������ȼ�
			uint16_t wPeirod;		//��ʱ������
		};
		uint32_t dwVal;
	};
	
	FUNC func;			//�ص�����
	uint8_t byID;		//��ʱ��������ID��
	uint8_t byType;		//��������
	
	uint8_t res[2];
}Queue_element_t;


typedef struct
{
	uint8_t byFront;	//����
	uint8_t byRear;		//��β
	uint8_t res[2];		//Ԥ������
	Queue_element_t List[TASK_QUEUE_MAX_SIZE];	//���л����
}Task_queue_t;



void manager_init(void);
bool manager_enque_isr(Queue_element_t *pTask);
void manager_scan(void);

void task_enter_critical(void);
void task_exit_critical(void);

#endif