#include <iostream>

using namespace std;
//Ҫ����C���ԵĿ��ļ� ��Ҫ��__Cpulsplus
#ifdef __cplusplus
extern "C"
{
#endif
	#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif
#pragma comment(lib,"avcodec.lib")//��ӿ��ļ���Ҳ���������Դ����
int main()
{
	//��ʾffmpeg�ı�������
	cout << "Test FFmpeg " << endl;

	getchar();
	cout << avcodec_configuration() << endl;

	system("pause");
	return 0;
}
