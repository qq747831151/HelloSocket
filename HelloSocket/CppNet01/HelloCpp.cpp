#include <stdio.h>
#include <functional>
void funA(int a)
{
	printf("funA\n");
}
void funB()
{
	printf("funB\n");
}

int main()
{
	//void(*p)();
	//p = funB;
	//p();

	//std::function<void(int)> call = funA;//���ƺ���ָ��
	//call(10);

	std::function<int(int)> call1;
	int n = 5;
	//�������� []�ⲿ���������б�
	call1 = [n](int a)->int//()�ǲ�����  ->int�Ƿ������� û�з���ֱֵ��ȥ��->int 
	{
		//������
		printf("funA%d\n", n + a);
		printf("%d\n", n);
		return n + a;

	};
	 n = 7;
	int b = call1(10);
	printf("%d\n", b);
	return 0;

}
/* lambda���ʽ  ��������ʽ ��������
[ caputrue ] ( params ) opt -> ret { body; };

[ �ⲿ���������б� ] ( ������ ) ��������� -> ����ֵ���� { ������; };

�����б�lambda���ʽ�Ĳ����б�ϸ������lambda���ʽ�ܹ����ʵ��ⲿ�������Լ���η�����Щ������

1) []�������κα�����

2) [&]�����ⲿ�����������б���������Ϊ�����ں�������ʹ�ã������ò��񣩡�

3) [=]�����ⲿ�����������б���������Ϊ�����ں�������ʹ��(��ֵ����)��

4) [=, &foo]��ֵ�����ⲿ�����������б������������ò���foo������

5) [bar]��ֵ����bar������ͬʱ����������������

6) [this]����ǰ���е�thisָ�룬��lambda���ʽӵ�к͵�ǰ���Ա����ͬ���ķ���Ȩ�ޡ�
����Ѿ�ʹ����&���� = ����Ĭ�Ϻ��д�ѡ�
����this��Ŀ���ǿ�����lamda��ʹ�õ�ǰ��ĳ�Ա�����ͳ�Ա������

////////
1).capture�ǲ����б�

2).params�ǲ�����(ѡ��)

3).opt�Ǻ���ѡ�������mutable,exception,attribute��ѡ�

mutable˵��lambda���ʽ���ڵĴ�������޸ı�����ı��������ҿ��Է��ʱ�����Ķ����non-const������

exception˵��lambda���ʽ�Ƿ��׳��쳣�Լ������쳣��

attribute�����������ԡ�

4).ret�Ƿ���ֵ���͡�(ѡ��)

5).body�Ǻ����塣
*/