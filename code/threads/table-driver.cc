#include "table-driver.h"
#include "system.h"
#include <cstdio>

void tableInsert(Table& table, int size, int which)
{
	char temp = 'a';
	char *m;
	int index;
	//����֮������ͨ�������������Ƿ�һ��
	for (int i = 0; i < size; i++)
	{
		char* element = new char(temp + i);
		index=table.Alloc(reinterpret_cast<void*>(element));
		printf("[Thread %d] insert %c to index %d\n", which,*element,index);
		m = reinterpret_cast<char*>(table.Get(index));
		printf("table[%d]:%c\n",index,*m);
	}
}

void tableRemove(Table& table, int size, int which)
{
	int k=size,i=0;
	while(k)
	{
		char *m=reinterpret_cast<char*>(table.Get(i));
		if(m!=NULL)
		{
			table.Release(i);
			printf("[Thread %d] remove index %d:%c\n", which, i,*m);
			k--;
		}
		i++;
	}
}
