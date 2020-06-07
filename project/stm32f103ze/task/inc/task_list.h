#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"


#define TASK_MAX_SIZE		TASK_MAX_NUM		//���Ļ�����������
#define TASK_MEMORY_SIZE	(TASK_MAX_SIZE + 1)	//�����ڴ����
#define MAX_PRIORITY		0xFF				//������ȼ�
#define MAX_TASK_ID			0xFF				//�������ID
#define TASK_MAX_UNIQUE_ID	32					//ΨһID��Χ


//�������
#define TASK_GUARTD_INDEX		TASK_MAX_SIZE
#define TASK_GUARTD_ID			MAX_TASK_ID
#define TASK_GUARTD_PRIORITY	0

#if (1 == TASK_DEBUG_ON)
	#define TASK_PRINT(x)	printf x
#else
	#define TASK_PRINT(x)
#endif



typedef struct
{
	FUNC CallBack;		//������
	uint8_t byPriority;	//�������ȼ�   1 -> 32    �����ȼ� -> �����ȼ�
	uint8_t byID;		//����ID
	uint8_t byNext;		//��һ������λ��
	uint8_t res[1];		//Ԥ������
}Task_node_t;

typedef struct 
{
	Task_node_t List[TASK_MEMORY_SIZE];	//�����¼���
	bool bMemFlag[TASK_MAX_SIZE];	//�����¼����ڴ��п��б�־λ
	bool bIDFlag[TASK_MAX_UNIQUE_ID];	//Ψһ�����־λ
	uint8_t byMemIndex;				//�ڴ��¼���ţ�����ǰ���ڴ���У�����һ���ڴ�����Ҳ�ǿ���
	uint8_t byNum;					//�����¼������¼��ĸ���
	uint8_t byHead;					//�����¼�����Ԫ��
}Task_list_t;

typedef struct
{
	void (*init)(void);
	void (*priority_increase)(void);
	void (*push)(Task_node_t *pTask);
	FUNC (*pop)(void);
	void (*print)(void);
}Task_t;


void task_list_init(void);
bool task_list_pop(void);
bool task_list_push(Task_node_t *pTask, bool bReplace);
void task_list_delete(uint8_t byID);
void task_list_print(void);

#endif

