/********************************************************
*file name: task_manager.c
*function:  ��Ҫ�������жϺ��������������й�������ʵ���ٽ���Դ�Ļ������
			�ٽ���Դ��Ҫ��ָ��ʱ�������������ȼ�������������ͳ�ƣ���������
			�����Ҫ���жϺ�������Ҫ������������и��ģ�����ʵ��ΪA���������ȰѺ���A�������������У�
			Ȼ���ں�̨������ִ��A����ʵ�ֶ���������ĸ��ġ�
*ʵ�֣�		���ݽṹ���ö��еķ�ʽʵ�֣�����к������ж��е��ã����Ӻ����ں�̨�����е��ã�������ת��������������
			ͬ������ת���ķ�ʽʵ���ٽ���Դ�Ļ������
			
*note: 		����������ã������жϺ����ж�ʱ�����������ͨ��������ɾ��
			�����ٽ����Դ
********************************************************/
#include <stdio.h>
#include "task_manager.h"
#include "timer_list.h"
#include "task_list.h"
#include "terminal.h"

//�ڲ���������
static volatile Task_queue_t stQueue;	//�������
volatile uint8_t critical_nesting = 0;	//�ٽ�������

//�ڲ���������
static bool manager_deque(Queue_element_t *pTask);
static void manager_queue_init(void);

/*********************************************************
* Name		: manager_init
* Function	: �����������ʼ��
* Input		: None
* Output	: None
* Return	: None
* Note		: None 
*********************************************************/
void manager_init(void)
{
	timer_list_init();
	task_list_init();

}

/*********************************************************
* Name		: manager_queue_init
* Function	: ������г�ʼ��
* Input		: None
* Output	: None
* Return	: None
* Note		: �ú������Ϊ���ж���ִ�еĺ��� 
*********************************************************/
static void manager_queue_init(void)
{
	memset((uint8_t *)&stQueue, 0, sizeof(Task_queue_t));
}

/*********************************************************
* Name		: manager_enque_isr
* Function	: ��������У����ж���ʹ��
* Input		: Queue_element_t *pTask		������Ϣָ��
* Output	: None
* Return	: true  ����гɹ�   false�����ʧ��
* Note		: �ú������Ϊ���ж���ִ�еĺ��� 
*********************************************************/
bool manager_enque_isr(Queue_element_t *pTask)
{
	uint8_t byNewRear = (stQueue.byRear + 1) % TASK_QUEUE_MAX_SIZE;
	if(byNewRear == stQueue.byFront)
	{
		return false;
	}
	
	stQueue.List[stQueue.byRear].dwVal = pTask->dwVal;
	stQueue.List[stQueue.byRear].func = pTask->func;
	stQueue.List[stQueue.byRear].byType = pTask->byType;
	stQueue.List[stQueue.byRear].byID = pTask->byID;
	
	stQueue.byRear = byNewRear;
	return true;
}

/*********************************************************
* Name		: manager_deque
* Function	: ���������
* Input		: Queue_element_t *pTask ��������ָ��
* Output	: None
* Return	: true ���ӳɹ�   false ����ʧ��
* Note		: None 
*********************************************************/
static bool manager_deque(Queue_element_t *pTask)
{
	if(stQueue.byRear == stQueue.byFront)
	{
		return false;
	}
	
	pTask->dwVal = stQueue.List[stQueue.byFront].dwVal;
	pTask->func = stQueue.List[stQueue.byFront].func;
	pTask->byType = stQueue.List[stQueue.byFront].byType;
	pTask->byID = stQueue.List[stQueue.byFront].byID;
	
	stQueue.byFront = (stQueue.byFront + 1) % TASK_QUEUE_MAX_SIZE;
	return true;
}

/*********************************************************
* Name		: manager_scan
* Function	: ��������У��ں�̨ʹ��,�������е������������Ӧ��������
* Input		: None
* Output	: None
* Return	: None
* Note		: ��Ҫ����ѭ�������� 
*********************************************************/
void manager_scan(void)
{
	Queue_element_t que_task;
	Task_node_t pTask;
	Timer_node_t timer;
	while(manager_deque(&que_task))
	{
		if(que_task.byType == TASK_TYPE_TIMER)
		{
			timer.CallBack = que_task.func;
			timer.wPeriod = que_task.wPeirod;
			timer.byRepeat = que_task.byRepeat;
			timer.byID = que_task.byID;
			timer_list_push(&timer);
		}
		else
		{
			pTask.CallBack = que_task.func;
			pTask.byPriority = que_task.byPriority;
			pTask.byID = que_task.byID;
			task_list_push(&pTask, true);
		}
	}
}

/*********************************************************
* Name		: task_enter_critical
* Function	: �����ٽ��
* Input		: None
* Output	: None
* Return	: None
* Note		: ��task_exit_critical()�����ɶ�ʹ��  
*********************************************************/
void task_enter_critical(void)
{
	DISABLE_INT();
	critical_nesting++;
}

/*********************************************************
* Name		: task_exit_critical
* Function	: �˳��ٽ��
* Input		: None
* Output	: None
* Return	: None
* Note		: ��task_enter_critical()�����ɶ�ʹ�� 
*********************************************************/
void task_exit_critical(void)
{
	critical_nesting--;
	if(0 == critical_nesting)
	{
		ENABLE_INT();
	}
}
