#include "table.h"


Table::Table(int size)
{
	tableLock = new Lock("tablelock");
	tableSize = size;
	table = reinterpret_cast<void**>(new char* [size]);		//ָ���С��һ������������ָ����Ը�ֵ��void*���������Ƿ�
	for (int i = 0; i < size; i++)
		table[i] = NULL;
}

Table::~Table()
{
	delete tableLock;
	delete[]  (char*)table;
}

int Table::Alloc(void * object)
{
	tableLock->Acquire();
	for (int i = 0; i < tableSize; i++)
	{
		//�ҵ�һ����ָ����в���
		if (table[i] == NULL)
		{
			table[i] = object;
			tableLock->Release();
			return i;
		}
	}
	tableLock->Release();
	return -1;
}

// return the object from table index 'index' or NULL on error.
// (assert index is in range).  Leave the table entry allocated
// and the pointer in place.
void * Table::Get(int index)
{
	ASSERT(index <= tableSize - 1)
	return table[index];
}

void Table::Release(int index)
{
	tableLock->Acquire();
	if (index < 0 || index >= tableSize)
	{
		printf("the index is out of range");
	}
	else
	{
		//����objecһ����֪��delete������������
		table[index] = NULL;
	}
	tableLock->Release();
}
