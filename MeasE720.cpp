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

// Запись значения смещения из файла Excel
void MeasE720::setBias(int b)
{
    Bias = b;
    //Bias = b / 1000;
}

// Запись значения смещения из прибора
void MeasE720::setBias(unsigned char b1, unsigned char b2)
{
  Bias = (b1 + b2 * 256) * 10;
  //Bias = (double)(b1 + b2 * 256) / 100;
}

// Чтение значения смещения
int MeasE720::getBias(void) const

//double MeasE720::getBias(void) const
{
  return Bias;
  //return Bias * 1000;
}

// Запись значения основного параметра измеряемого сигнала из файла Excel
void MeasE720::setPrimaryMeasValue(double pmv)
{
  PrimaryMeasValue = pmv;
}

// Вычисление основного параметра измеряемого сигнала
void MeasE720::setPrimaryMeasValue(unsigned char ch1, unsigned char ch2,
  unsigned char ch3, unsigned char ch4)
{
  double PrimaryDegree; // Показатель степени измеряемой величины

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

// Получить значение основного параметра измеряемого сигнала
double MeasE720::getPrimaryMeasValue(void) const
{
  return PrimaryMeasValue;
}

// вычисление и запись дополнительного измеряемого сигнала из прибора
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
// Вычисление и запись дополнительного измеряемого сигнала из файла Excel
void MeasE720::setSecondMeasValue(double smv)
{
  SecondMeasValue = smv;
}
// Чтение значения дополнительного измеряемого сигнала
double MeasE720::getSecondMeasValue(void) const
{
  return SecondMeasValue;
}

// Установка уровня измерительного сигнала из файла
void MeasE720::setValueMeasSignal(int vms)
{
  ValueMeasSignal = vms;
}

// Установка значения уровня измерительного сигнала из прибора
void MeasE720::setValueMeasSignal(unsigned char vms)
{
  ValueMeasSignal = static_cast<int>(vms) / 100;
}
// Чтение уровня измерительного сигнала
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

// Запись флага режимов, полученный из прибора
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

// Чтение флага режимов, полученный из прибора
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

// Запись байта режима работы прибора
void MeasE720::setModeWork(unsigned char mw)
{
  ModeWork = mw;
}

// Чтение байта режима работы прибора
unsigned char MeasE720::getModeWork(void) const
{
  return ModeWork;
}

// Запись байта измеряемого параметра
void MeasE720::setMeasParam(unsigned char mp)
{
  MeasParam = mp;
}

// Чтение байта измеряемого параметра
unsigned char MeasE720::getMeasParam(void) const
{
  return MeasParam;
}

// Запись байта дополнительного измеряемого параметра
void MeasE720::setSecMeasParam(unsigned char smp)
{
  SecMeasParam = smp;
}

// Чтение байта дополнительного измеряемого параметра
unsigned char MeasE720::getSecMeasParam(void) const
{
  return SecMeasParam;
}

// Запись основного измеряемого параметра (типа Rs) из файла
void MeasE720::setMeasParamBasic(int mpb)
{
  MeasParamBasic = mpb;
}

// Чтение основного измеряемого параметра (типа Rs)
int MeasE720::getMeasParamBasic(void) const
{
  return MeasParamBasic;
}

// Запись дополнительного измеряемого параметра (типа D) из файла
void MeasE720::setMeasParamSecond(int mps)
{
  MeasParamSecond = mps;
}

// Чтение дополнительного измеряемого параметра (типа D)
int MeasE720::getMeasParamSecond(void) const
{
  return MeasParamSecond;
}

// Чтение значения флага режима работы автоматического режима
// переключения диапазонов
unsigned char MeasE720::getAutoModeSwitchSubBands(void) const
{
  return AutoModeSwitchSubBands;
}

// Чтение значения флага режима работы параллельной/
// последовательной схемы замещения
unsigned char MeasE720::getParSerEqCirc(void) const
{
  return ParSerEqCirc;
}

// Чтение значения флага режима работы допуска
unsigned char MeasE720::getAdmission(void) const
{
  return Admission;
}

// Чтение значения флага режима работы автовыбора схемы замещения
unsigned char MeasE720::getAutoSelectEqCirc(void) const
{
  return AutoSelectEqCirc;
}
