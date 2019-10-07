#ifndef TIMER_TASK_H
#define TIMER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

//���֧�ֵĶ�ʱ����������
#define TIMER_MAX_SIZE		TIMER_MAX_NUM
#define TIMER_MEMORY_SIZE	(TIMER_MAX_SIZE + 1)
#define MAX_PERIOD			0xFFFF
#define MAX_TIMER_ID		0xFF
#define MAX_REPEAT			0xFF


#define TIMER_MAX_UNIQUE_ID		32
//����byRepeat��ֵΪ0XFF����־Ϊһֱ���ڵ�����
#define TIMER_CONITNUOUS_REPEAT MAX_REPEAT
#define TIMER_CONTINUOUS_VAL	(MAX_REPEAT - 1)

//�������
#define TIMER_GUARTD_INDEX		TIMER_MAX_NUM
#define TIMER_GUARTD_ID			MAX_TIMER_ID
#define TIMER_GUARTD_PERIOD		MAX_PERIOD
#define TIMER_GUARTD_REPEAT		MAX_REPEAT

#if (1 == TIMER_DEBUG_ON)
	#define TIMER_PRINT(x)	printf x
#else
	#define TIMER_PRINT(x)
#endif

typedef struct
{
	FUNC CallBack;			//��ʱ���ص�
	uint16_t wTick;			//��ʱ��ֵ
	uint16_t wPeriod;		//��������
	uint8_t byRepeat;			//�ظ�����
	uint8_t byID;		//�����־��
	uint8_t byNext;			//��һ����ʱ������λ��
	uint8_t res[1];
}Timer_node_t;

typedef struct
{
	Timer_node_t List[TIMER_MEMORY_SIZE];	//��ʱ�������
	bool bMemFlag[TIMER_MAX_SIZE];			//�����¼����ڴ��п��б�־λ
	bool bIDFlag[TIMER_MAX_SIZE];			//Ψһ��ʱ����־λ
	uint8_t byMemIndex;						//�ڴ��¼���ţ�����ǰ���ڴ���У�����һ���ڴ�����Ҳ�ǿ���
	uint8_t byNum;							//�����¼������¼��ĸ���
	uint8_t byHead;							//�����¼�����Ԫ��
	uint8_t byMutex;							//�����ź���
}Timer_list_t;



void timer_list_init(void);
void timer_list_pop(void);
void timer_list_push(Timer_node_t *pNode);
bool timer_list_empty(void);
void timer_list_delete(uint8_t byID);
void timer_list_print(void);

#endif