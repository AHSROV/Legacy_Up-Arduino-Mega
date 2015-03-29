#include <Ethernet.h>
#include <EthernetClient.h>
#include <SPI.h>

//Program Description
//Full of Swag - Mebin

char msgBuffer[15];
int charsInBuffer=0;
char ethernetMsgBuffer[15];
int iE=0;

int motorPin[] = {
  9, 8, 7, 6, 5, 3};
int directionPin[] = {
  A0, A1, A2, A3, A4, A5};
int ff1Pin[] = {
  21,23,25,27,33,35};
int ff2Pin[] ={
  22,24,26,32,34,36};
int motorValue[6] = {
  0, 0, 0, 0, 0, 0};
boolean directionState[8] = {
  false, false, false, false, false, false};

EthernetClient client; 

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 
  192, 168, 1, 178 };
byte server[] = { 
  192, 168, 1, 177 };

boolean serverConnected;

void setup()
{
  Ethernet.begin(mac, ip);

  Serial.begin(9600);

  if (client.connect(server, 23)) {
    Serial.println("connected");
    serverConnected = true;
  } 
  else 
  {
    Serial.println("e: connection failed");
   serverConnected = false;
  }
  SetPinModes();
}

void loop()
{
  if(!client.connected())
  {
    if(serverConnected)
    {
      serverConnected == false;
      Serial.println("e: arduino down disconnected");
    }

    if (client.connect(server, 23)) {
      Serial.println("connected");
      serverConnected = true;
    } 
  }

  if (Serial.available() > 0)
  {  
    //these Major Leters will be used for controlling the motor.
    //Read the Data Taken and then converts it into Byte from ASCII Code
    if (charsInBuffer >= sizeof(msgBuffer))
    {
      Serial.println("e: overflow");
      charsInBuffer = 0; 
    } 
    msgBuffer[charsInBuffer] = (byte)Serial.read();
    charsInBuffer++;  

    // Wait for end of line
    if (msgBuffer[charsInBuffer-1] == 10 || msgBuffer[charsInBuffer-1] == 13)
    {
      if(msgBuffer[0] == 'm' && msgBuffer[1] <= '7')
      {
        ProcessMotorCommand(msgBuffer, charsInBuffer);
      }
      else if(msgBuffer[0] == 'p' && msgBuffer[1] == 'u')
      {
        Serial.println("pu");
      }
      else
      {
        msgBuffer[charsInBuffer - 1] = 0;
        client.println(msgBuffer);
      }
      charsInBuffer = 0; 
    } 
  } 

  // Get the string from Ethernet
  if(client.available() > 0)  
  {
    if (iE >= sizeof(ethernetMsgBuffer))
    {
      Serial.println("e: Ethernet overflow");
      iE = 0; 
    }
    ethernetMsgBuffer[iE] = (byte)client.read();
    iE++;

    if(ethernetMsgBuffer[iE-1] == 10 || ethernetMsgBuffer[iE-1] == 13)
    {
      ethernetMsgBuffer[iE-1] = 0;
      //Serial.print("Ethernet incoming: ");
      Serial.println(ethernetMsgBuffer);
      iE = 0;
    }
  }
}

int GetValFromString(char *p, int length)
{
  if (!isDigit(*p) || length < 1)
  {
    return -1;
  }

  int v=0;
  while(isDigit(*p) && length > 0)
  {
    v = v*10 + *p - '0';
    p++; 
    length--;
  }
  return v;
}

void ProcessMotorCommand(char msgBuffer[], int msgLen)
{ 
  if (msgLen < 4)
  {
    Serial.println("e: cmd too short.");
    return;
  }
  int motor = GetValFromString(&msgBuffer[1], 1);
  if ((motor < 0) || (motor > 5))
  {
    Serial.println("e: invalid motor.");
    return;
  }

  //if it starts with M and ends with a % and a LF or CR ... DO what it says below
  //Motor Direction Decleration
  if(msgBuffer[2]=='f')
  {
    // We assume everything but the first letter and the line ending are part of a number.
    int speed = GetValFromString(&msgBuffer[3], charsInBuffer-3);
    if (speed < 0)
    {
      Serial.println("e: error, not an integer.");
      return;
    }
    SendMessage(speed,motor,true);  
  }   
  else if(msgBuffer[2]=='r')
  {
    int speed = GetValFromString(&msgBuffer[3], charsInBuffer-3);
    SendMessage(speed,motor,false);  
  }
  else if(msgBuffer[2]=='q')
  {
    ErrorReport(ff1Pin[motor], ff2Pin[motor], motor);
  }
  else
  {
    Serial.print("e: Unrecognized cmd: ");
    msgBuffer[charsInBuffer-1] = 0;
    Serial.println(msgBuffer);
  }

  Serial.println();
}


int SendMessage(int speed, int motor, boolean forward)
{
  if(speed>=0 || speed<=100)
  {
    motorValue[motor] = (int)(speed*2.55);
    Serial.print("Speed: ");
    Serial.println(motorValue[motor]);
  }

  directionState[motor] = forward;
  Serial.print("Direction: ");
  Serial.println(directionState[motor]);

  UpdateMotorSpeed(motor);
  UpdateMotorDirection(motor);
}

void SetPinModes()
{
  UpdateMotorSpeeds();
  UpdateMotorDirections();

  for(int i = 0; i < 6; i++)
  {
    pinMode(motorPin[i], OUTPUT);
    pinMode(directionPin[i], OUTPUT);
    pinMode(ff1Pin[i], INPUT);
    pinMode(ff2Pin[i], INPUT);
  }
}

void UpdateMotorSpeeds() 
{
  for(int i = 0; i < 6; i++) 
  {
    UpdateMotorSpeed(i);
  }
}

void UpdateMotorSpeed(int i) 
{
  analogWrite(motorPin[i], motorValue[i]);
  Serial.print("Pin ");
  Serial.print(motorPin[i]);
  Serial.print(" set to ");  
  Serial.println(motorValue[i]);
}

void UpdateMotorDirections()
{
  for(int i = 0; i < 6; i++)
  {
    digitalWrite(directionPin[i], directionState[i]);
  }
}

void UpdateMotorDirection(int i)
{
  digitalWrite(directionPin[i], directionState[i]);
}

int ErrorReport(int ff1Pin,int ff2Pin, int motor)
{
  Serial.print("m");
  Serial.print(motor);
  Serial.print("q");
  if(digitalRead(ff1Pin) == HIGH && digitalRead(ff2Pin) == HIGH)
  {
    Serial.println(3);
  } 
  else if(digitalRead(ff1Pin) == LOW && digitalRead(ff2Pin) == HIGH)
  {
    Serial.println(2);
  }
  else if(digitalRead(ff1Pin) == HIGH && digitalRead(ff2Pin) == LOW)
  {
    Serial.println(1);
  }
  else
  {
    Serial.println(0); 
  } 
}






