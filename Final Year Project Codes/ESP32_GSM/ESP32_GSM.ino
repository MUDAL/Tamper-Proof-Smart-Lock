
void SendSMS(char* message)
{
  Serial2.println("AT+CMGF=1");
  delay(500);
  Serial2.println("AT+CMGS=\"+2349058041373\"");
  delay(500);
  Serial2.println(message);
  delay(500);
  Serial2.write(26); //Command termination
  delay(500);
}

void setup() 
{
  // put your setup code here, to run once:
  Serial2.begin(9600,SERIAL_8N1,-1,17);
  SendSMS("Go home");
  delay(5000);
  SendSMS("Get some rest");
  delay(5000);
  SendSMS("How are you?");
  delay(5000);
  SendSMS("Take care of yourself");
}

void loop() 
{
  // put your main code here, to run repeatedly:
}
