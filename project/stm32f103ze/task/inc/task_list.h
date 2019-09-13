#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include "task_config.h"

#define TASK_MAX_SIZE		TASK_MAX_NUM
#define TASK_MEMORY_SIZE	(TASK_MAX_SIZE + 1)


typedef struct
{
	FUNC call_back;		//������
	uint8_t priority;	//�������ȼ�   1 -> 32    �����ȼ� -> �����ȼ�
	uint8_t next;		//��һ������λ��
	uint8_t res[2];		//Ԥ������
}Task_node_t;

typedef struct 
{
	Task_node_t list[TASK_MEMORY_SIZE];	//�����¼���
	bool mem_flag[TASK_MAX_SIZE];	//�����¼����ڴ��п��б�־λ
	uint8_t mem_index;				//�ڴ��¼���ţ�����ǰ���ڴ���У�����һ���ڴ�����Ҳ�ǿ���
	uint8_t size;					//�����¼������¼��ĸ���
	uint8_t head;					//�����¼�����Ԫ��
}Task_list_t;

typedef struct
{
	void (*init)(void);
	void (*priority_increase)(void);
	void (*push)(Task_node_t *ptask);
	FUNC (*pop)(void);
	void (*print)(void);
}Task_t;

void task_list_init(void);
void task_list_priority_increase(void);
FUNC task_list_pop(void);
void task_list_push(Task_node_t *ptask);
void task_list_print(void);


#endif