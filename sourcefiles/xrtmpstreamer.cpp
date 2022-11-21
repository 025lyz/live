/*******************************************************************************
**                                                                            **
**                     Jiedi(China nanjing)Ltd.                               **
**	               �������Ĳܿ����˴��������Ϊѧϰ�ο�                       **
*******************************************************************************/

/*****************************FILE INFOMATION***********************************
**
**  Project       : FFmpegSDKʵս�γ�
**  Description   : ����FFMpegSDK��ʵս�γ�
**  Contact       : xiacaojun@qq.com
**  ����   : http://blog.csdn.net/jiedichina
**  ��Ƶ�γ�
**  http://edu.csdn.net/lecturer/lecturer_detail?lecturer_id=961
**  http://edu.51cto.com/lecturer/12016059.html
**  http://study.163.com/u/xiacaojun
**  https://jiedi.ke.qq.com/
**  ����γ̺���Լ�Ⱥ��ѯ�γ�ѧϰ����
**  Ⱥ�� 132323693 fmpeg��ֱ�������γ�
**  ΢�Ź��ں�  : jiedi2007
**	ͷ����	 : �Ĳܿ�
**
*******************************************************************************/

#include "xrtmpstreamer.h"
#include "XController.h"
#include <iostream>
//#include "XController.h"
using namespace std;
static bool isStream = false;
XRtmpStreamer::XRtmpStreamer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}
void XRtmpStreamer::Stream()
{
	cout << "Stream";
	if (isStream)
	{
		isStream = false;
		ui.startButton->setText(QString::fromLocal8Bit("��ʼ"));
		XController::Get()->Stop();
	}
	else
	{
		isStream=true;
		ui.startButton->setText(QString::fromLocal8Bit("ֹͣ"));
		QString url = ui.inUrl->text();
		bool ok = false;
		int camIndex = url.toInt(&ok);
		if (!ok)
		{
			XController::Get()->inUrl = url.toStdString();
		}
		else
		{
			XController::Get()->camIndex = camIndex;
		}
		XController::Get()->outUrl = ui.outUrl->text().toStdString();
		XController::Get()->Set("d", (ui.face->currentIndex() + 1) * 3 );
		XController::Get()->Start();
	}

}