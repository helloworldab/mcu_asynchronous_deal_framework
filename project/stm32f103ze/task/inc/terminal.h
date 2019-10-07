#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

#define RECV_BUFF_MAX_SIZE			99
#define RECV_BUFF_ARRAY_SIZE		(RECV_BUFF_MAX_SIZE + 1)
#define FUN_ARGUMENTS_MAX_SIZE		5


#define RECV_BUFF_OVERFLOW			32		

#define	INPUT_KEY_UP			33
#define INPUT_KEY_DOWN			34
#define INPUT_KEY_RIGHT			35	
#define INPUT_KEY_LEFT			36



#define CHAR_TO_SPECIAL(x)			((x) - 65 + 33)

typedef struct
{
	void *func;
	const char *pName;
	const uint8_t byParamterNum;
}Function_map_t;




typedef enum
{
	TERMINAL_IDEL = 0x01,	//����״̬,�ȴ������һ���ַ�
	TERMINAL_BUSY,			//������,�ȴ��������
	TERMINAL_READY,			//�����˿��Ʒ����ȴ���������
}Terminal_state_t;

typedef struct
{
	void (*OutPutCallBack)(uint8_t byData);		//��ӡ���ݻص�����
	char byRecvBuff[RECV_BUFF_ARRAY_SIZE];		//���ջ�����
	uint8_t byRecvLen;							//���ճ���
	uint8_t byRecvLast;							//���յ����ݻ������
	uint8_t byCtrlType;							//�����ַ�����
	uint8_t bySpecialCharFlag;					//������Ʒ��ű�־
	volatile Terminal_state_t eState;			//����״̬����״̬��־
}Terminal_t;


void terminal_input_predeal(uint8_t byData);
void terminal_handler(void);
void terminal_init(void);

#endif