//---------------------------------------------------------------------------

#include <vcl.h>
#include <registry.hpp>
#include <math.h>
#include <DateUtils.hpp>
#include <windows.h>
#pragma hdrstop

#include "UE720.h"
#include "MeasE720.cpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormE720 *FormE720;
MeasE720 DeviceParamMeas;
AnsiString Parameter[15] = {"Cp","Lp","Rp","Gp","Bp","|Y|","Q","Cs","Ls",
  "Rs","f","Xs","|Z|","D","I"};
//---------------- Конструктор --------------------------------
__fastcall TFormE720::TFormE720(TComponent* Owner)
  : TForm(Owner)
{
  hndPort = NULL;  
  fileNameFlags = "flags.dat";
  fileNameParam = "mode.dat";
  fileNameResult = "result.dat";
  fileNameArrayMeas = "arraymeas.dat";
  
  for (int i = 0; i < numberByte; i++)
  {
    arrayMeasure[i] = 0;
  }
  
  for (int i = 0; i < numberValue; i++)
  {
    arrayFirstValue[i] = 0;
    arraySecondValue[i] = 0;
  }
  
  ListParam = new TStringList;
  ListResult = new TStringList;
  ListFlag = new TStringList;
  ListResultTime = new TStringList;
}

void __fastcall TFormE720::NMinClick(TObject *Sender)
{
  FormE720->ClientWidth = 368;
  FormE720->ClientHeight = 232;
  NMin->Checked = true;
  NMax->Checked = false;
}

void __fastcall TFormE720::NMaxClick(TObject *Sender)
{
  FormE720->ClientWidth = 600;
  FormE720->ClientHeight = 500;
  NMin->Checked = false;
  NMax->Checked = true;
}

void __fastcall TFormE720::tbExitClick(TObject *Sender)
{
  if(Application->MessageBoxA("Вы действительно хотите выйти из программы?",
    "Выход из программы Е720",
    MB_YESNOCANCEL + MB_ICONINFORMATION) == IDYES)
  {
    FormE720->Close();
  }
}

void __fastcall TFormE720::FormDestroy(TObject *Sender)
{

  ListFlag->Free();
  ListFlag = NULL;
  ListParam->Free();
  ListParam = NULL;
  ListResult->Free();
  ListResult = NULL;
  ListResultTime->Free();
  ListResultTime = NULL;

  if (hndPort != NULL)
  {
    CloseHandle(hndPort);
    hndPort = NULL;
  }
}

void __fastcall TFormE720::GetPort(void)
{
  if (hndPort != NULL)
  {
    CloseHandle(hndPort);
    hndPort = NULL;
  }


  TRegistry* Reg = new TRegistry;
  TStringList* List = new TStringList;
  AnsiString Port;

  Reg->RootKey = HKEY_LOCAL_MACHINE;
  Reg->OpenKey("HARDWARE\\DEVICEMAP\\SERIALCOMM", false);
  Reg->GetValueNames(List);

  for (int i = 0; i < List->Count; i++)
  {
    Port = Reg->ReadString(List->Strings[i]).SetLength(3);
    if(Port.SetLength(3) == "COM")
    {
      cbInPort->Items->Add(Reg->ReadString(List->Strings[i]));
    }
  }

  if (List->Count == 0)
  {
    sbOutInfo->Panels->Items[0]->Text = "Внутренний порт не найден!";
    cbInPort->Enabled = false;
    cbHandlePort->Enabled = true;
  }

  List->Free();
  Reg->CloseKey();
  Reg->Free();
  cbInPort->ItemIndex = 0;
}

void __fastcall TFormE720::FormCreate(TObject *Sender)
{
  GetPort();
}

bool __fastcall TFormE720::CreatePort(AnsiString port)
{
  if (hndPort != NULL)
  {
    CloseHandle(hndPort);
    hndPort = NULL;
  }

  hndPort = CreateFile(port.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL,
    OPEN_EXISTING, 0, 0);

  if (!GetCommState(hndPort, &lpDcb))
  {
    MessageDlg("Не могу открыть порт", mtError, TMsgDlgButtons() << mbOK, 0);
    return false;
  }


  lpDcb.BaudRate = CBR_9600;
  lpDcb.ByteSize = 8;
  lpDcb.Parity = NOPARITY;
  lpDcb.StopBits = ONESTOPBIT;

  if (!SetCommState(hndPort, &lpDcb))
  {
    MessageDlg("Значения параметров недопустимы!", mtError,
      TMsgDlgButtons() << mbOK, 0);
    return false;
  }
  cto.ReadIntervalTimeout = 0;
  cto.ReadTotalTimeoutConstant = 500;
  cto.ReadTotalTimeoutMultiplier = 500;
  cto.WriteTotalTimeoutConstant = 500;
  cto.WriteTotalTimeoutMultiplier = 500;

  if (!SetCommTimeouts(hndPort, &cto))
  {
    MessageDlg("Недопустимые временные интервалы!", mtError,
      TMsgDlgButtons() << mbOK, 0);
    return false;
  }
  return true;
}

void __fastcall TFormE720::cbInPortClick(TObject *Sender)
{
  TComboBox* cb = (TComboBox *) Sender;

  switch (cb->ItemIndex)
  {
    case 0:
    {
      tbStart->Enabled = false;
      if (hndPort != NULL)
      {
        CloseHandle(hndPort);
        hndPort = NULL;
      }
      break;
    }
    case 1: case 2: case 3: case 4: case 5: case 6:
    {
      if (CreatePort(cb->Text))
      {
        sbOutInfo->Panels->Items[1]->Text = "Порт: " + cb->Text;
        tbStart->Enabled = true;
      }
      else
      {
        sbOutInfo->Panels->Items[0]->Text = "Не удалось открыть порт " +
          cb->Text;
      }
      break;
    }
  }  
}

int __fastcall TFormE720::ReadFileFlags(AnsiString fName)
{
  int res = 0;

  ListFlag->Clear();

  try
  {
    ListFlag->LoadFromFile(fName);
    try
    {
      res = StrToInt(ListFlag->Strings[0]);
      return res;
    }
    catch(...)
    {
      Application->MessageBoxA("Неверный формат данных!", "E720",
        MB_OK + MB_ICONERROR);
      WriteFileFlags(fileNameFlags, 3);
      return 0;
    }
  }
  catch(...)
  {
    Application->MessageBoxA("Ошибка открытия файла!", "E720",
      MB_OK + MB_ICONERROR);
    return 0;
  }
}

void __fastcall TFormE720::WriteFileFlags(AnsiString fName, int num)
{
  ListFlag->Clear();  
  ListFlag->Add(IntToStr(num)); 

  try
  {
    ListFlag->SaveToFile(fName);
  }
  catch(...)
  {
    Application->MessageBoxA("Ошибка записи в файл!", "E720",
      MB_OK + MB_ICONERROR);
  }
}

void __fastcall TFormE720::tbStartClick(TObject *Sender)
{
  int flag = ReadFileFlags(fileNameFlags);
  cbInPort->Enabled = false;
  cbHandlePort->Enabled = false;

  switch(flag)
  {
    case 0:
    {
      Application->MessageBoxA("Проверьте файл управления режимами!",
        "E720", MB_OK + MB_ICONERROR);
      return;
    }

    case 1:
    {
      sbOutInfo->Panels->Items[0]->Text = "Идут измерения...";
      break;
    }

    case 2:
    {
      tbStop->Enabled = true;
      indPower = true;
      
      Measure();
      break;
    }

    case 3:
    {
      sbOutInfo->Panels->Items[0]->Text = "Измерения завершены!";
      TimerCount->Enabled = false;
      tbStop->Enabled = true;
      
      TimerMeas->Enabled = true;
      break;
    }
  }
}

void __fastcall TFormE720::Measure(void)
{  
  ReadFileParam(fileNameParam);
  ReadFrame();
  CalcResult();
  CompareData();

  for (int i = 0; i < numberValue; i++)
  {
    arrayFirstValue[i] = 0;
    arraySecondValue[i] = 0;
  }
  GenFirstValue = 0;
  GenSecondValue = 0;

  for(int i = 0; i < ValueMeasure; i++)
  {
    ReadFrame();
    CalcResult();

    if(i < ValueSkip) continue;

    arrayFirstValue[i - ValueSkip] = DeviceParamMeas.getPrimaryMeasValue();
    arraySecondValue[i - ValueSkip] = DeviceParamMeas.getSecondMeasValue();
    GenFirstValue += arrayFirstValue[i - ValueSkip];
    GenSecondValue += arraySecondValue[i - ValueSkip];
  }
    GenFirstValue /= (ValueMeasure - ValueSkip);
    GenSecondValue /= (ValueMeasure - ValueSkip);
    ListResult->Clear();
    ListResult->Add(FloatToStrF(GenFirstValue, ffExponent, 4, 15));
    ListResult->Add(FloatToStrF(GenSecondValue, ffExponent, 4, 15));
    CreateListResult();
    WriteFileFlags(fileNameFlags,3);
    WriteToFileResults(fileNameResult);
    tbStart->Click();
}

void __fastcall TFormE720::ReadFileParam(AnsiString fName)
{
  ListParam->Clear();

  try
  {
    ListParam->LoadFromFile(fName);
  }
  catch(...)
  {
    Application->MessageBoxA("Ошибка открытия файла!", "E720",
      MB_OK + MB_ICONERROR);
    return;
  }  
}

void __fastcall TFormE720::ReadFrame(void)
{
  unsigned char bb;
  DWORD jj;

  countBuffer = 0;
  sbOutInfo->Panels->Items[0]->Text = "Чтение кадра данных";

  PurgeComm(hndPort, PURGE_TXCLEAR|PURGE_RXCLEAR);
  do
  {
    do
    {
      if(indPower == false)
      {
        if(hndPort != NULL)
        {
          CloseHandle(hndPort);
          hndPort = NULL;
        }
        return;
      }

      if (!ReadFile(hndPort, &bb, 1, &jj, NULL))
      {
        ShowMessage("Ошибка чтения кадра данных: " + myMessageError());
        return;
      }
      else
      {
        Application->ProcessMessages();
      }
    }while(jj != 1);
  }while(bb != 0xAA);

  arrayMeasure[countBuffer] = bb;
  countBuffer++;

  do
  {
    do
    {
      if(!ReadFile(hndPort, &bb, 1, &jj, NULL))
      {
        ShowMessage("Ошибка ReadFrame: " + myMessageError());
        return;
      }
      else
      {
        Application->ProcessMessages();
      }
    }while(jj != 1);
    arrayMeasure[countBuffer] = bb;
    countBuffer++;
  }while(countBuffer != numberByte);
}

AnsiString __fastcall TFormE720::myMessageError(void)
{
  return SysErrorMessage(GetLastError());
}

void __fastcall TFormE720::CalcResult(void)
{
  DeviceParamMeas.setBias(arrayMeasure[1], arrayMeasure[2]);
  DeviceParamMeas.setValueMeasSignal((int)(arrayMeasure[3]));
  DeviceParamMeas.setFrequence(arrayMeasure[4], arrayMeasure[5],
    arrayMeasure[6]);
  DeviceParamMeas.setFlags(arrayMeasure[7]);
  DeviceParamMeas.setModeWork(arrayMeasure[8]);
  DeviceParamMeas.setLimit(arrayMeasure[9]);
  DeviceParamMeas.setMeasParamBasic(arrayMeasure[10]);
  DeviceParamMeas.setMeasParamSecond(arrayMeasure[11]);
  DeviceParamMeas.setPrimaryMeasValue(arrayMeasure[16], arrayMeasure[17],
    arrayMeasure[18], arrayMeasure[19]);
  DeviceParamMeas.setSecondMeasValue(arrayMeasure[12], arrayMeasure[13],
    arrayMeasure[14], arrayMeasure[15]);
}

void __fastcall TFormE720::CompareData(void)
{
  int fBias = StrToInt(ListParam->Strings[0]);
  int fVms = StrToInt(ListParam->Strings[1]);
  int fFreq = StrToInt(ListParam->Strings[2]);
  int fPmp = StrToInt(ListParam->Strings[4]);
  int fSmp = StrToInt(ListParam->Strings[5]);
  ValueMeasure = StrToInt(ListParam->Strings[6]);
  IntervalMeas = StrToInt(ListParam->Strings[7]);
  ValueSkip = StrToInt(ListParam->Strings[8]);
  int dBias = (DeviceParamMeas.getBias());
  int dVms = DeviceParamMeas.getValueMeasSignal() * 10;
  int dFreq = DeviceParamMeas.getFrequence();
  int dPmp = DeviceParamMeas.getMeasParamBasic();
  int dSmp = DeviceParamMeas.getMeasParamSecond();
  while(dBias != fBias) {
    dBias = DeviceParamMeas.getBias();
    if(indPower == false) break;
    CompareBias(dBias, fBias);
  }
  CompareMeasValue(dVms, fVms);
  while(dFreq != fFreq) {
    if(indPower == false) return;
    CompareFrequence(dFreq, fFreq);
    dFreq = DeviceParamMeas.getFrequence();
  }
}

bool __fastcall TFormE720::RangeBias(int Val)
{
  for(int i = 0; i < 11; i++)
  {
    if(Val == aBias[i]) return true;
  }
  return false;
}

void __fastcall TFormE720::CompareBias(int dBias, int fBias)
{
  if(dBias != fBias)
  {
    SendByteToPort(mBias);
    if(dBias > fBias)
    {
      if(CompareRangeBias(dBias, fBias))
      {
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias)
          {
            if(indPower == false) return;
            if(dBias < fBias) return;
            btnLeft->Click();
            ReadFrame();            
            CalcResult();     
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }

        if(((RangeBias(dBias) == true) && (RangeBias(fBias) == false)) ||
          ((RangeBias(dBias) == false) && (RangeBias(fBias) == false)) ||
          ((RangeBias(dBias) == false) && (RangeBias(fBias) == true)))
          {
            while(dBias != fBias)  // >
            {
              if(indPower == false) return;
              if(dBias < fBias) return;
              SendByteToPort(mDown);
              ReadFrame();           
              CalcResult();          
              dBias = DeviceParamMeas.getBias();
            }
            return;
          }
      }

      else
      {
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias)  // >
          {
           
            if(indPower == false)
            {
              sbOutInfo->Panels->Items[0]->Text = "Остановлено пользователем!";
              return;
            }
           
            if(dBias < fBias) return;
            
            while(!CompareRangeBias(dBias, fBias))
            {
              
              if(indPower == false)
              {
                sbOutInfo->Panels->Items[0]->Text = "Остановлено пользователем!";
                return;
              }
             
              if(dBias < fBias) return;
              SendByteToPort(mLeft);
              ReadFrame();
              CalcResult();
              dBias = DeviceParamMeas.getBias();
            }
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
      
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == false))
        {
          for(int i = 11; i > 0; i--)
          {
            if(dBias == aBias[i])
            {
              SendByteToPort(mLeft);  
              ReadFrame();           
              CalcResult();          
              dBias = DeviceParamMeas.getBias();
              if(CompareRange(dBias, fBias))
                break;
            }
          }

          while(dBias != fBias)
          {            
            if(indPower == false) return;
            if(dBias < fBias) return;
            SendByteToPort(mDown);  
            ReadFrame();            
            CalcResult();           
            dBias = DeviceParamMeas.getBias();  
          }
          return;
        }
        
        if((RangeBias(dBias) == false) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias)  // >
          {
            if(indPower == false) return;
            if(dBias < fBias) return;
            SendByteToPort(mLeft);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }

        
        if((RangeBias(dBias) == false) && (RangeBias(fBias) == false))
        {
          while(!CompareRangeBias(dBias, fBias))
          {
            if(indPower == false) return;
            if(dBias < fBias) return;
            SendByteToPort(mLeft);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          
          while(dBias != fBias)  
          {
            if(indPower == false) return;
            if(dBias < fBias) return;
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
        }
      }
    }
    
    if(dBias < fBias)
    {
      if(CompareRangeBias(dBias, fBias))
      {
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias) // <
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
        
        if(((RangeBias(dBias) == true) && (RangeBias(fBias) == false)) ||
          ((RangeBias(dBias) == false) && (RangeBias(fBias) == false)) ||
          ((RangeBias(dBias) == false) && (RangeBias(fBias) == true)))
        {
          while(dBias != fBias)  // <
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
      }
     
      else
      {
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias)  // <
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            
            while(!CompareRangeBias(dBias, fBias))
            {
              if(indPower == false) return;
              if(dBias > fBias) return;
              SendByteToPort(mRight);
              ReadFrame();
              CalcResult();
              dBias = DeviceParamMeas.getBias();
            }
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
       
        if((RangeBias(dBias) == true) && (RangeBias(fBias) == false))
        {
          while(dBias != fBias)
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            while(!CompareRangeBias(dBias, fBias))
            {
              if(indPower == false) return;
              if(dBias > fBias) return;
              SendByteToPort(mRight);
              ReadFrame();
              CalcResult();
              dBias = DeviceParamMeas.getBias();
            }
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
       
        if((RangeBias(dBias) == false) && (RangeBias(fBias) == true))
        {
          while(dBias != fBias)  // <
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          return;
        }
        
        if((RangeBias(dBias) == false) && (RangeBias(fBias) == false))
        {
          while(!CompareRangeBias(dBias, fBias))
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
          while(dBias != fBias)  // <
          {
            if(indPower == false) return;
            if(dBias > fBias) return;
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dBias = DeviceParamMeas.getBias();
          }
        }
      }
    }
  }
}

void __fastcall TFormE720::SendByteToPort(unsigned char ch)
{
  unsigned int j;
  DWORD jj;

  j = GetTickCount();
  while(GetTickCount() < (j + 400))
  {
    Application->ProcessMessages();
  }
 
  if(hndPort != NULL)
  {
    if(!WriteFile(hndPort, &ch, 1, &jj, NULL))
    {
      ShowMessage("Ошибка SendByteToPort: " + myMessageError());
      return;
    }
    else
      m->Lines->Add("SendToPort!");
  }
}


bool __fastcall TFormE720::CompareRange(int d, int f)
{
  for (int i = 0; i < 16; i++)
  {
    if(((d >= aFrequence[i]) && (d <= aFrequence[i+1])) &&
      ((f >= aFrequence[i]) && (f <= aFrequence[i+1])))
    return true;
  }
  return false;
}

int __fastcall TFormE720::CompareFreqMax(int d, int f)
{
  if((d >= 25) && (d <= 1000) && (f < 1000))
    return freqMin;
  else
    return freqMax;
}

bool __fastcall TFormE720::CompareRangeBias(int d, int f)
{
  bool range = false;

  for(int i = 0; i < 10; i++)
  {
    if(((d >= aBias[i]) && (d <= aBias[i+1])) &&
      ((f >= aBias[i]) && (f <= aBias[i+1])))
    {
        range =  true;
    }
  }
  if(!range)
  {
    if(( d >= 4000) || (f <= 10000))
     range = true;
  }
  return range;
}

char __fastcall TFormE720::CompareBiasMax(int d, int f)
{
  if((d >= 0) && (d <= 6300) && (f < 6300))
    return 0x4;
  else
    return 0x9;
}

void __fastcall TFormE720::tbStopClick(TObject *Sender)
{
  TimerMeas->Enabled = false;
  TimerCount->Enabled = false;
  TimerDelay->Enabled = false;
  tbStart->Enabled = true;
  indPower = false;
  cbInPort->Enabled = true;
  cbHandlePort->Enabled = true;
  sbOutInfo->Panels->Items[0]->Text = "Измерения остановлены!";
}

void __fastcall TFormE720::CompareMeasValue(int DMeasValue, int fMeasValue)
{
  const int ch = 10;

  if(DMeasValue != fMeasValue)
  {

    SendByteToPort(mValue);

    if(DMeasValue < fMeasValue)
    {
      while(DMeasValue < fMeasValue)
      {
        DMeasValue += ch;
        SendByteToPort(mUp);
        if(DMeasValue > fMeasValue) return;
      }
    }
    if(DMeasValue > fMeasValue)
    {
      while(DMeasValue > fMeasValue)
      {
        DMeasValue -= ch;
        SendByteToPort(mDown);
        if(DMeasValue < fMeasValue) return;
      }
    }
  }
}

bool __fastcall TFormE720::RangeFreq(int Val)
{
  for(int i = 0; i < 17; i++)
  {
    if(Val == aFrequence[i]) return true;
  }

  return false;
}

void __fastcall TFormE720::CompareFrequence(int dFreq, int fFreq)
{
  if(dFreq != fFreq)
  {
    SendByteToPort(mFreq);

    if(dFreq > fFreq)
    {
      if(CompareRange(dFreq, fFreq))
      {
        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            SendByteToPort(mLeft);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if(((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == false)) ||
          ((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == false)) ||
          ((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == true)))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }
      }

      else
      {
        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;

            while(!CompareRange(dFreq, fFreq))
            {
              if(indPower == false) return;
              if(dFreq < fFreq) return;
              SendByteToPort(mLeft);
              ReadFrame();
              CalcResult();
              dFreq = DeviceParamMeas.getFrequence();
            }
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == false))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            while(!CompareRange(dFreq, fFreq))
            {
              if(indPower == false) return;
              if(dFreq < fFreq) return;
              SendByteToPort(mLeft);
              ReadFrame();
              CalcResult();
              dFreq = DeviceParamMeas.getFrequence();
            }
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            SendByteToPort(mLeft);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == false))
        {
          while(!CompareRange(dFreq, fFreq))
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            SendByteToPort(mLeft);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }

          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq < fFreq) return;
            SendByteToPort(mDown);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
        }
      }
    }

    if(dFreq < fFreq)
    {
      if(CompareRange(dFreq, fFreq))
      {
        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if(((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == false)) ||
          ((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == false)) ||
          ((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == true)))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }
      }

      else
      {
        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == true) && (RangeFreq(fFreq) == false))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            while(!CompareRange(dFreq, fFreq))
            {
              if(indPower == false) return;
              if(dFreq > fFreq) return;
              SendByteToPort(mRight);
              ReadFrame();
              CalcResult();
              dFreq = DeviceParamMeas.getFrequence();
            }
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == true))
        {
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          return;
        }

        if((RangeFreq(dFreq) == false) && (RangeFreq(fFreq) == false))
        {
          while(!CompareRange(dFreq, fFreq))
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mRight);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
          while(dFreq != fFreq)
          {
            if(indPower == false) return;
            if(dFreq > fFreq) return;
            SendByteToPort(mUp);
            ReadFrame();
            CalcResult();
            dFreq = DeviceParamMeas.getFrequence();
          }
        }
      }
    }
  }
}

void __fastcall TFormE720::WriteToFileResults(AnsiString fName)
{
  try
  {
    ListResult->SaveToFile(fName);
  }
  catch(...)
  {
    Application->MessageBoxA("Ошибка записи в файл!", "E720",
      MB_OK + MB_ICONERROR);
  }
}

void __fastcall TFormE720::WriteToFileAllResult(AnsiString fName)
{
  try
  {
    ListResultTime->SaveToFile(fName);
  }
  catch(...)
  {
    Application->MessageBoxA("Ошибка записи в файл!", "E720",
      MB_OK + MB_ICONERROR);
  }
}

void __fastcall TFormE720::btnFreqClick(TObject *Sender)
{
  SendByteToPort(mFreq);
}

void __fastcall TFormE720::btnLevelClick(TObject *Sender)
{
  SendByteToPort(mValue);
}

void __fastcall TFormE720::btnLeftClick(TObject *Sender)
{
  SendByteToPort(mLeft);
}

void __fastcall TFormE720::btnRightClick(TObject *Sender)
{
  SendByteToPort(mRight);  
}

void __fastcall TFormE720::btnDownClick(TObject *Sender)
{
  SendByteToPort(mDown);  
}

void __fastcall TFormE720::btnUpClick(TObject *Sender)
{
  SendByteToPort(mUp);  
}

void __fastcall TFormE720::btnEnterClick(TObject *Sender)
{
  SendByteToPort(mEnter);  
}

void __fastcall TFormE720::btnMenuClick(TObject *Sender)
{
  SendByteToPort(mMenu);
}

void __fastcall TFormE720::btnLimitClick(TObject *Sender)
{
  SendByteToPort(mBand);
}

void __fastcall TFormE720::btnCClick(TObject *Sender)
{
  SendByteToPort(mC);
}

void __fastcall TFormE720::btnLClick(TObject *Sender)
{
  SendByteToPort(mL);
}

void __fastcall TFormE720::btnRClick(TObject *Sender)
{
  SendByteToPort(mR);
}

void __fastcall TFormE720::btnIClick(TObject *Sender)
{
  SendByteToPort(mI);  
}

void __fastcall TFormE720::btnZClick(TObject *Sender)
{
  SendByteToPort(mZ);
}

void __fastcall TFormE720::btnBiasClick(TObject *Sender)
{
  SendByteToPort(mBias);  
}

void __fastcall TFormE720::btnCalibrationClick(TObject *Sender)
{
  SendByteToPort(mkalibr);  
}

void __fastcall TFormE720::getTimeDate(void)
{
  mHour = HourOf(Now());
  mMinutes = MinuteOf(Now());
  mDay = DayOf(Now());
  mMonth = MonthOf(Now());
  mYear = YearOf(Now());
}

void __fastcall TFormE720::cbWinClick(TObject *Sender)
{
  if(cbWin->Checked)
    FormE720->FormStyle = fsStayOnTop;
  else
    FormE720->FormStyle = fsNormal;
}

void __fastcall TFormE720::CreateListResult(void)
{  
  AnsiString str = "";
  const AnsiString split = " ";
  ReadFileAllResult(fileNameArrayMeas);
  getTimeDate();
  str = "";

  str += IntToStr(mDay);
  if(mMonth < 10)
    str += ".0" + IntToStr(mMonth) + "." + IntToStr(mYear);
  else
    str += "." + IntToStr(mMonth) + "." + IntToStr(mYear);

 
  str += split + IntToStr(mHour);
  if(mMinutes < 10)
    str += ":0" + IntToStr(mMinutes);
  else
    str += ":" + IntToStr(mMinutes);

  
  str += split + FloatToStr(DeviceParamMeas.getBias());  
  str += split + IntToStr(DeviceParamMeas.getValueMeasSignal()*10);  
  str += split + IntToStr(DeviceParamMeas.getFrequence());  
  str += split + IntToStr(DeviceParamMeas.getModeWork()); 
  str += split + IntToStr(DeviceParamMeas.getLimit()); 
  str += split + Parameter[DeviceParamMeas.getMeasParamBasic()];  
  str += split + Parameter[DeviceParamMeas.getMeasParamSecond()];  
  str += split + IntToStr(ValueMeasure);  
  str += split + IntToStr(ValueSkip);  
  ListResultTime->Add(str);
  str = "";
  str += FloatToStrF(GenFirstValue, ffExponent, 4, 15);

  for (int i = 0; i < ValueMeasure; i++)
  {
    if(i < ValueSkip) continue;
    str += split + FloatToStrF(arrayFirstValue[i - ValueSkip], ffExponent,4,15);
  }

  ListResultTime->Add(str);
  str = "";
  str += FloatToStrF(GenSecondValue, ffExponent, 4, 15);

  for(int i = 0; i< ValueMeasure; i++)
  {
    if(i < ValueSkip) continue;
    str += split + FloatToStrF(arraySecondValue[i - ValueSkip], ffExponent,4,15);
  }
  ListResultTime->Add(str);

  WriteToFileAllResult(fileNameArrayMeas);
}

void __fastcall TFormE720::ReadFileAllResult(AnsiString fName)
{
  try
  {
    ListResultTime->LoadFromFile(fName);
  }
  catch(...)
  {
    Application->MessageBoxA("Не могу открыть файл!", "E720",
      MB_OK + MB_ICONERROR);
  }
}

void __fastcall TFormE720::TimerMeasTimer(TObject *Sender)
{
  TimerMeas->Enabled = false;
  tbStart->Click();
}

void __fastcall TFormE720::tbtnClearClick(TObject *Sender)
{
m->Clear();
}


