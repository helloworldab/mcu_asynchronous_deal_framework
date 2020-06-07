#include <string.h>
#include "list.h"
#include "debug.h"

/*********************************************************
* Name		: statck_init
* Function	: ջ��ʼ��
* Input		: uint8_t *pStack	ջ�ڴ��ַ
			  uint8_t bySize	ջ�Ŀռ��С
* Output	: None
* Return	: None
* Note		: None
*********************************************************/
void stack_init(uint8_t *pStack, uint8_t bySize)
{
	memset(pStack, 0, bySize);
	pStack[STACK_CAPACITY] = bySize - 1;
	pStack[STACK_POINT] = STACK_BUTTON;
}
/*********************************************************
* Name		: stack_push
* Function	: ջԪ����ջ
* Input		: uint8_t *pStack	ջ�ڴ��ַ
			  uint8_t byData	��ջԪ��
* Output	: None
* Return	: true ��ջ�ɹ�  false ��ջʧ��
* Note		: None
*********************************************************/
bool stack_push(uint8_t *pStack, uint8_t byData)
{
	ERROR("stack full", pStack[STACK_POINT] < pStack[STACK_CAPACITY], return false);
	pStack[++pStack[STACK_POINT]] = byData;
	return true;
}

/*********************************************************
* Name		: stack_pop
* Function	: ��ջ
* Input		: uint8_t *pStack	ջ�ڴ��ַ
* Output	: uint8_t *pData	��ջԪ��
* Return	: true ��ջ�ɹ�  false ջΪ�գ���ջʧ��
* Note		: None
*********************************************************/
bool stack_pop(uint8_t *pStack, uint8_t *pData)
{
	ERROR("stack empty", pStack[STACK_POINT] > STACK_BUTTON,return false);
	*pData = pStack[pStack[STACK_POINT]--]; 
	return true;
}
/*********************************************************
* Name		: stack_empty
* Function	: ���ջ�Ƿ�Ϊ��
* Input		: uint8_t *pStack	ջ�ڴ��ַ
* Output	: None
* Return	: true ջΪ��  false ջ�ǿ�
* Note		: None
*********************************************************/
bool stack_empty(uint8_t *pStack)
{
	return pStack[STACK_POINT] == STACK_BUTTON;
}

/*********************************************************
* Name		: stack_full
* Function	: ���ջ�ռ��Ƿ���
* Input		: uint8_t *pStack	ջ�ڴ��ַ
* Output	: None
* Return	: true ջ��  false ջ����
* Note		: None
*********************************************************/
bool stack_full(uint8_t *pStack)
{
	return pStack[STACK_POINT] == pStack[STACK_CAPACITY];
}
