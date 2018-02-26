#include "MeasE720.h"

MeasE720::MeasE720()
{ 
  ParSerEqCirc = 0;
  Admission = 0;
  AutoSelectEqCirc = 0;
}


double MeasE720::Pow(double x)
{
  return exp(x * log(10));
}

void MeasE720::setFrequence(int f)
{
  Frequence = f;
}

void MeasE720::setFrequence(unsigned char b1, unsigned char b2, unsigned char b3)
{
  Frequence = b1 + b2 * 256;
  Frequence = ceil(Frequence * Pow(b3));
}

int MeasE720::getFrequence(void) const
{
  return Frequence;
}

// ������ �������� �������� �� ����� Excel
void MeasE720::setBias(int b)
{
    Bias = b;
    //Bias = b / 1000;
}

// ������ �������� �������� �� �������
void MeasE720::setBias(unsigned char b1, unsigned char b2)
{
  Bias = (b1 + b2 * 256) * 10;
  //Bias = (double)(b1 + b2 * 256) / 100;
}

// ������ �������� ��������
int MeasE720::getBias(void) const

//double MeasE720::getBias(void) const
{
  return Bias;
  //return Bias * 1000;
}

// ������ �������� ��������� ��������� ����������� ������� �� ����� Excel
void MeasE720::setPrimaryMeasValue(double pmv)
{
  PrimaryMeasValue = pmv;
}

// ���������� ��������� ��������� ����������� �������
void MeasE720::setPrimaryMeasValue(unsigned char ch1, unsigned char ch2,
  unsigned char ch3, unsigned char ch4)
{
  double PrimaryDegree; // ���������� ������� ���������� ��������

  if((ch3 & 128) != 0)
  {
    PrimaryMeasValue = 0 - (ch1 ^ 255) - ((ch2 ^ 255) + (ch3 ^ 255) * 256) * 256;
  }
  else
  {
    PrimaryMeasValue = ch1 + (ch2 + ch3 * 256) * 256;
  }

  if((ch4 & 128) != 0)
  {
    PrimaryDegree = ch4 ^ 255;
    PrimaryDegree = -1 - PrimaryDegree;
  }
  else
  {
    PrimaryDegree = ch4;
  }

  PrimaryMeasValue = PrimaryMeasValue * Pow(PrimaryDegree);
}

// �������� �������� ��������� ��������� ����������� �������
double MeasE720::getPrimaryMeasValue(void) const
{
  return PrimaryMeasValue;
}

// ���������� � ������ ��������������� ����������� ������� �� �������
void MeasE720::setSecondMeasValue(unsigned char ch1, unsigned char ch2,
  unsigned char ch3, unsigned char ch4)
{
  double SecondDegree;

  if((ch3 & 128) != 0)
  {
    SecondMeasValue = 0 - (ch1 ^ 255) - ((ch2 ^ 255) + (ch3 ^ 255) * 256) * 256;
  }
  else
  {
    SecondMeasValue = ch1 + (ch2 + ch3 * 256) * 256;
  }
  if((ch4 & 128) != 0)
  {
    SecondDegree = ch4 ^ 255;
    SecondDegree = -1 - SecondDegree;
  }
  else
  {
    SecondDegree = ch4;
  }

  SecondMeasValue = SecondMeasValue * Pow(SecondDegree);
}
// ���������� � ������ ��������������� ����������� ������� �� ����� Excel
void MeasE720::setSecondMeasValue(double smv)
{
  SecondMeasValue = smv;
}
// ������ �������� ��������������� ����������� �������
double MeasE720::getSecondMeasValue(void) const
{
  return SecondMeasValue;
}

// ��������� ������ �������������� ������� �� �����
void MeasE720::setValueMeasSignal(int vms)
{
  ValueMeasSignal = vms;
}

// ��������� �������� ������ �������������� ������� �� �������
void MeasE720::setValueMeasSignal(unsigned char vms)
{
  ValueMeasSignal = static_cast<int>(vms) / 100;
}
// ������ ������ �������������� �������
int MeasE720::getValueMeasSignal(void) const
{
  return ValueMeasSignal;
}

void MeasE720::setLimit(int l)
{
  Limit = l;
}

void MeasE720::setLimit(unsigned char l)
{
  Limit = l;
}

int MeasE720::getLimit(void) const
{
  return Limit;
}

// ������ ����� �������, ���������� �� �������
void MeasE720::setFlags(unsigned char f)
{
  ModeFlag = f;
  const unsigned char f1 = 0x1;
  const unsigned char f2 = 0x2;
  const unsigned char f3 = 0x4;
  const unsigned char f4 = 0x8;

  AutoModeSwitchSubBands = f & f1;
  if(AutoModeSwitchSubBands == f1)
    AutoModeSwitchSubBands = 1;
  else
    AutoModeSwitchSubBands = 0;

  ParSerEqCirc = f & f2;
  if (ParSerEqCirc == f2)
    ParSerEqCirc = 1;
  else
    ParSerEqCirc = 0;

  Admission = f & f3;
  if(Admission == f3)
    Admission = 1;
  else
    Admission = 0;

  AutoSelectEqCirc = f & f4;
  if(AutoSelectEqCirc == f4)
    AutoSelectEqCirc = 1;
  else
    AutoSelectEqCirc = 0;
}

// ������ ����� �������, ���������� �� �������
int MeasE720::getFlags(void) const
{
  return ModeFlag;
}

void MeasE720::setFlagsEdit(unsigned char fe)
{
  EditFlag = fe;
}

int MeasE720::getFlagsEdit(void) const
{
  return EditFlag;
}

// ������ ����� ������ ������ �������
void MeasE720::setModeWork(unsigned char mw)
{
  ModeWork = mw;
}

// ������ ����� ������ ������ �������
unsigned char MeasE720::getModeWork(void) const
{
  return ModeWork;
}

// ������ ����� ����������� ���������
void MeasE720::setMeasParam(unsigned char mp)
{
  MeasParam = mp;
}

// ������ ����� ����������� ���������
unsigned char MeasE720::getMeasParam(void) const
{
  return MeasParam;
}

// ������ ����� ��������������� ����������� ���������
void MeasE720::setSecMeasParam(unsigned char smp)
{
  SecMeasParam = smp;
}

// ������ ����� ��������������� ����������� ���������
unsigned char MeasE720::getSecMeasParam(void) const
{
  return SecMeasParam;
}

// ������ ��������� ����������� ��������� (���� Rs) �� �����
void MeasE720::setMeasParamBasic(int mpb)
{
  MeasParamBasic = mpb;
}

// ������ ��������� ����������� ��������� (���� Rs)
int MeasE720::getMeasParamBasic(void) const
{
  return MeasParamBasic;
}

// ������ ��������������� ����������� ��������� (���� D) �� �����
void MeasE720::setMeasParamSecond(int mps)
{
  MeasParamSecond = mps;
}

// ������ ��������������� ����������� ��������� (���� D)
int MeasE720::getMeasParamSecond(void) const
{
  return MeasParamSecond;
}

// ������ �������� ����� ������ ������ ��������������� ������
// ������������ ����������
unsigned char MeasE720::getAutoModeSwitchSubBands(void) const
{
  return AutoModeSwitchSubBands;
}

// ������ �������� ����� ������ ������ ������������/
// ���������������� ����� ���������
unsigned char MeasE720::getParSerEqCirc(void) const
{
  return ParSerEqCirc;
}

// ������ �������� ����� ������ ������ �������
unsigned char MeasE720::getAdmission(void) const
{
  return Admission;
}

// ������ �������� ����� ������ ������ ���������� ����� ���������
unsigned char MeasE720::getAutoSelectEqCirc(void) const
{
  return AutoSelectEqCirc;
}
