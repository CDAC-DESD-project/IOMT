/*
 *
 * Created: 20-July-17 7:04:09 PM
 * Author : Mohd. Rayyan Akthar
 */

#include <Wearable.h>

#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>

#include <DS3231.h>

#define RS 2
//#define RW 1 // RW Pin of LCD is connected to ground.
#define E 3

DS3231  rtc(SDA, SCL);  // initialising constructor for IIC.
Time medicineTime;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A3, A2, A1, A0}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int firstindication = 8;		// Pin 8 for LED as first medicine indicator
int secondindication = 9;		// Pin 9 for LED as first medicine indicator

char input = '\0';
unsigned char flag1 = 0;		
unsigned char flag2 = 0;

void setup() {
  // put your setup code here, to run once:
  rtc.begin();					// initialise the RTC module.
  port_init();					// making arduino pins connected to LCD as output pins.
  lcd_init_4bit();				// initialising LCD as 4-bit mode.
  lcd_clr();					// clear the LCD for first time.
  introMessage();				// Initialising message
}

void loop() {
  // put your main code here, to run repeatedly:
 welcomeMessage();
 checkTime1Correctness();
 checkTime2Correctness();
 checkRTCTimeCorrectness();
 checkRTCDateCorrectness();
 checkAppointmentTimeCorrectness();
  // reading RTC
  medicineTime = rtc.getTime();
  
  char key = '\0';
  while ((key = keypad.getKey()) == 0)
  {
	  compare();
  }
  
  if (key) {
    flag1 =0 ; flag2 = 0;
	
    switch (key)					// A to set the first medicine timings; B to set the second medicine timings; C to set the RTC timings; D to set Doctor Appointment.
    {
      case 'A': lcd_clr();
        enterTime1();
        break;

      case 'B': lcd_clr();
        enterTime2();
        break;

      case 'C': lcd_clr();
        setrtcdate();
        break;

      case 'D': lcd_clr();
        setappointmentdate();
        break;

      default:  convertRTCTimeForLcd();
        break;
    }
  }
}

unsigned int unpackedBCDtoInt(int L1, int L2)
{
  char l1 = EEPROM.read(L1);
  char l2 = EEPROM.read(L2);
  unsigned int value = ((l1 - 48) * 10) + (l2 - 48);
  return value;
}

unsigned int unpackedyearBCDtoInt(int L1, int L2, int L3, int L4)
{
  char l1 = EEPROM.read(L1);
  char l2 = EEPROM.read(L2);
  char l3 = EEPROM.read(L3);
  char l4 = EEPROM.read(L4);
  unsigned int value = ((l1 - 48) * 1000) + ((l2 - 48)*100) + ((l3 - 48)*10) + (l4 - 48);
  return value;
}

void configMessage()
{
  lcd_const_str_wrt("Press D: To Ent");
  next_line();
  lcd_const_str_wrt("in config mode");
  delay(1000);
  return;
}


void port_init(void)
{
	// LCD Pins
	setbit(DDRD,DDD7);
	setbit(DDRD,DDD6);
	setbit(DDRD,DDD5);
	setbit(DDRD,DDD4);
	setbit(DDRD,DDD3);
	setbit(DDRD,DDD2);
	
	// Making output for indication
	setbit(DDRB,DDB0);  // PB0==> Pin8 Arduino
	setbit(DDRB,DDB1);	// PB1==> Pin9 Arduino
  
}

void lcd_init_4bit(void)
{
	lcd_cmd_wrt(0x33);
	lcd_cmd_wrt(0x32);
	lcd_cmd_wrt(0x28);
	lcd_cmd_wrt(0x80);
	lcd_cmd_wrt(0xE);
}

void lcd_data_wrt(unsigned char data)
{
	delay(2);
	PORTD = (data & 0xF0);
	data_wrt_en();
	PORTD = ((data & 0x0F)<<4);
	data_wrt_en();

}

void lcd_cmd_wrt(unsigned char data)
{
	delay(2);
	PORTD = (data & 0xF0);
	cmd_wrt_en();
	PORTD = ((data & 0x0F)<<4);
	cmd_wrt_en();
}

void cmd_wrt_en(void)
{
	clrbit(PORTD,RS);
	//clrbit(PORTD,RW);
	setbit(PORTD,E);
	delay(10);
	clrbit(PORTD,E);
}

void data_wrt_en(void)
{
	setbit(PORTD,RS);
	//clrbit(PORTD,RW);
	setbit(PORTD,E);
	delay(10);
	clrbit(PORTD,E);
}

void lcd_const_str_wrt(char *data)
{
	unsigned char i=0;
	while (data[i]!=0)
	{
		lcd_data_wrt(data[i++]);
		if(i==16)
		lcd_cmd_wrt(0xC0);
	}
}

void shift_disp_right(unsigned char times)
{
	unsigned char i=0;
	while(i<(times*40))
	{
		lcd_cmd_wrt(0x1C);
		delay(50);
		i++;
	}

}

void shift_disp_left(unsigned char times)
{
	unsigned char i=0;
	while(i<(times*40))
	{
		lcd_cmd_wrt(0x18);
		delay(50);
		i++;
	}
}

void next_line(void)
{
	lcd_cmd_wrt(0xC0);
}

void lcd_str_wrt(unsigned char data[])
{
	unsigned char i=0;
	while (data[i]!=0)
	{
		lcd_data_wrt(data[i++]);
		if(i==16)
		lcd_cmd_wrt(0xC0);
	}
}

void lcd_clr()
{
	lcd_cmd_wrt(0x01);
}

void goto_location(unsigned char x, unsigned char y)
{
	if(x==1)
	lcd_cmd_wrt((0x80+y));
	if(x==2)
	lcd_cmd_wrt((0xc0+y));
}

void inttostr(unsigned int x, unsigned char str[])
{
	unsigned int num, i=0;
	num = x;
	while(num!=0)
	{
		str[i] = (num%10)+48;
		num = num/10;
		i++;
	}
}

void introMessage()
{
	lcd_const_str_wrt("    CDAC DESD   ");
	lcd_cmd_wrt(LCD_CURSERBLINKOFF);
	delay(2000);
}

void welcomeMessage(void)
{
  lcd_clr();
  lcd_const_str_wrt("<WELCOME TO THE ");
  next_line();
  lcd_const_str_wrt(" WORLD OF IoMT>");
  lcd_cmd_wrt(LCD_CURSERBLINKOFF);
}

void enterTime1(void)
{
  flag1 = 0;
	lcd_clr();
  lcd_const_str_wrt("Enter 1st Timing");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Time in ");
  next_line();
  lcd_const_str_wrt("24hrs Format");
  checkResetTimer1Time();
  if(flag1 == 1 && flag2 == 1)			// terminates the execution of function again.
	  return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(TIME1H1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer1Time();
   if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME1H2, input);
  goto_location(1,2);
  lcd_const_str_wrt(":");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer1Time();
   if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME1M1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer1Time();
   if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME1M2, input);
  goto_location(1,5);
  lcd_const_str_wrt(":");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer1Time();
   if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME1S1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer1Time();
  if(flag1 == 1 && flag2 == 1)
   return;
  lcd_data_wrt(input);
  EEPROM.write(TIME1S2, input);

  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkResetTimer1Time();
   if(flag1 == 1 && flag2 == 1)
    return;
    flag1 = 1;
  while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("TIMER1 SET DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
    return;
  }
}

void enterTime2(void)
{
  flag1 = 0;
  lcd_clr();
  lcd_const_str_wrt("Enter 2nd Timing");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Time in ");
  next_line();
  lcd_const_str_wrt("24hrs Format");
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(TIME2H1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME2H2, input);
  goto_location(1,2);
  lcd_const_str_wrt(":");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer2Time();
 if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME2M1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME2M2, input);
  goto_location(1,5);
  lcd_const_str_wrt(":");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME2S1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(TIME2S2, input);

  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkResetTimer2Time();
  if(flag1 == 1 && flag2 == 1)
    return;
  flag1 = 1;
 while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("TIMER2 SET DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
    return;
  }
}

void setrtctimer(){
  flag1 = 0;
  lcd_clr();
  lcd_const_str_wrt("Enter RTC Timing");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Time in ");
  next_line();
  lcd_const_str_wrt("24hrs Format");
  delay(1000);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMEH1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMEH2, input);
  goto_location(1,2);
  lcd_const_str_wrt(":");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMEM1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMEM2, input);
  goto_location(1,5);
  lcd_const_str_wrt(":");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMES1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCTIMES2, input);
  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkKeyRTCTime();  
  if(flag1 == 1 && flag2 == 1)
    return;
  flag1 = 1;
  while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("RTC TIME SET");
    next_line();
    lcd_const_str_wrt("     DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
    rtc.setTime(unpackedBCDtoInt(RTCTIMEH1, RTCTIMEH2), unpackedBCDtoInt(RTCTIMEM1, RTCTIMEM2), unpackedBCDtoInt(RTCTIMES1, RTCTIMES2)); // 
    return;
  }
}

void setrtcdate()
{
  flag1 = 0;
  lcd_clr();
  lcd_const_str_wrt("Enter RTC Date");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Date in");
  next_line();
  lcd_const_str_wrt("DD:MM:YYYYFormat");
  delay(1000);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(RTCDATED1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATED2, input);
  goto_location(1,2);
  lcd_const_str_wrt("-");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEM1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEM2, input);
  goto_location(1,5);
  lcd_const_str_wrt("-");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEY1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEY2, input);
  goto_location(1,8);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEY3, input);
  goto_location(1,9);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(RTCDATEY4, input);
  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkKeyRTCDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  flag1 = 1;
  while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("RTC DATE SET");
    next_line();
    lcd_const_str_wrt("     DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
    rtc.setDate(unpackedBCDtoInt(RTCDATED1, RTCDATED2), unpackedBCDtoInt(RTCDATEM1, RTCDATEM2), unpackedyearBCDtoInt(RTCDATEY1, RTCDATEY2, RTCDATEY3, RTCDATEY4));
    setrtctimer();
  }
}

void setappointmentdate()
{
  flag1 = 0;
  lcd_clr();
  lcd_const_str_wrt("Enter");
  next_line();
  lcd_const_str_wrt("Appointment Date");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Date in");
  next_line();
  lcd_const_str_wrt("DD:MM:YYYYFormat");
  delay(1000);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(APPDATED1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATED2, input);
  goto_location(1,2);
  lcd_const_str_wrt("-");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();;
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEM1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEM2, input);
  goto_location(1,5);
  lcd_const_str_wrt("-");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEY1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEY2, input);
  goto_location(1,8);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEY3, input);
  goto_location(1,9);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPDATEY4, input);
  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkKeyAppointmentDate();
  if(flag1 == 1 && flag2 == 1)
    return;
  flag1 = 1;
  while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("Appointment date");
    next_line();
    lcd_const_str_wrt("Setting DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
	timeonserialport();
	setappointmenttime();
  }
}

void setappointmenttime()
{
  flag1 = 0;
  lcd_clr();
  lcd_const_str_wrt("Enter Appointment");
  next_line();
  lcd_const_str_wrt("Timings");
  delay(1000);
  lcd_clr();
  lcd_const_str_wrt("Enter Time in ");
  next_line();
  lcd_const_str_wrt("24hrs Format");
  delay(1000);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_clr();
  lcd_data_wrt(input);
  EEPROM.write(APPTIMEH1, input);
  goto_location(1,1);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPTIMEH2, input);
  goto_location(1,2);
  lcd_const_str_wrt(":");
  goto_location(1,3);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPTIMEM1, input);
  goto_location(1,4);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPTIMEM2, input);
  goto_location(1,5);
  lcd_const_str_wrt(":");
  goto_location(1,6);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPTIMES1, input);
  goto_location(1,7);
  lcd_cmd_wrt(LCD_DISPLAYON_CURSORBLINKING);
  checkKeyAppointmentTime();
  if(flag1 == 1 && flag2 == 1)
    return;
  lcd_data_wrt(input);
  EEPROM.write(APPTIMES2, input);
  next_line();
  lcd_const_str_wrt("Press D: To Set");
  checkKeyRTCTime();  
  if(flag1 == 1 && flag2 == 1)
    return;
  flag1 = 1;
  while(keypad.getKey() != 'D');
  {
    lcd_clr();
    lcd_const_str_wrt("APP. TIME SET");
    next_line();
    lcd_const_str_wrt("     DONE!");
    lcd_cmd_wrt(LCD_CURSERBLINKOFF);
    delay(2000);
    return;
  }	
}

void compare()
{
  medicineTime = rtc.getTime();
	if (medicineTime.hour == unpackedBCDtoInt(TIME1H1, TIME1H2) && medicineTime.min == unpackedBCDtoInt(TIME1M1, TIME1M2) && medicineTime.sec == unpackedBCDtoInt(TIME1S1, TIME1S2))
  {
	blinkingLedFirstMedicine();
  }

  if (medicineTime.hour == unpackedBCDtoInt(TIME2H1, TIME2H2) && medicineTime.min == unpackedBCDtoInt(TIME2M1, TIME2M2) && medicineTime.sec == unpackedBCDtoInt(TIME2S1, TIME2S2))
  {
    blinkingLedSecondMedicine();
  }
  
  if (unpackedBCDtoInt(APPDATED1, APPDATED2) == (medicineTime.date + 1) && medicineTime.mon == unpackedBCDtoInt(APPDATEM1, APPDATEM2) && medicineTime.year == unpackedyearBCDtoInt(APPDATEY1, APPDATEY2, APPDATEY3, APPDATEY4))
	 { 
	  medicineTime = rtc.getTime();
	  if(medicineTime.min == 00)
    indicationForAppointment();
  }
}
void blinkingLedFirstMedicine()
{
  input = '\0';
  unsigned char flag = 0;
  if(flag == 1)
    return;
	lcd_clr();
	lcd_const_str_wrt("Time for first");
	next_line();
	lcd_const_str_wrt("Medicine");
	while(1)
	{
		if((input = keypad.getKey()) == '*')
		{
		  flag = 1;
		  digitalWrite(firstindication, LOW);
      welcomeMessage();
		  break;
		}
    digitalWrite(firstindication, HIGH);
    delay(200);
    digitalWrite(firstindication, LOW);
    delay(200);
	}
}

void blinkingLedSecondMedicine()
{
  input = '\0';
  unsigned char flag = 0;
  if(flag == 1)
    return;
	lcd_clr();
	lcd_const_str_wrt("Time for Second");
	next_line();
	lcd_const_str_wrt("Medicine");
	while(1)
	{
		if((input = keypad.getKey()) == '*')
		{
		  flag = 1;
		  digitalWrite(secondindication, LOW);
      welcomeMessage();
		  break;
		}
    digitalWrite(secondindication, HIGH);
    delay(200);
    digitalWrite(secondindication, LOW);
    delay(200);
	}
}

void timeonserialport()
{
  Serial.print("Hr-");
  Serial.print(medicineTime.hour);
  Serial.print("min-");
  Serial.print(medicineTime.min);
  Serial.print("sec-");
  Serial.println(medicineTime.sec);
}

void checkResetTimer1Time()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	  int firstaddress = TIME1H1;
	  for(unsigned char i = 0; i<6; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++; 
	  }
	 flag1 = 1;
	 enterTime1();
  }
   if(flag1 == 1)
   flag2 = 1;
}

void checkResetTimer2Time()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	 int firstaddress = TIME2H1;
	  for(unsigned char i = 0; i<6; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++; 
	  } 
	 flag1 = 1;
	enterTime2();
  }
   if(flag1 == 1)
   flag2 = 1;
}

void checkKeyRTCTime()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	  int firstaddress = RTCTIMEH1;
	  for(unsigned char i = 0; i<6; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++; 
	  }
   flag1 = 1;
   setrtctimer();
  }
   if(flag1 == 1)
   flag2 = 1;
}

void checkKeyRTCDate()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	  int firstaddress =RTCDATED1;
	  for(unsigned char i = 0; i<8; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++; 
	  }
   flag1 = 1;
   setrtcdate();
  }
   if(flag1 == 1)
   flag2 = 1;
}

void checkKeyAppointmentDate()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	  int firstaddress = APPDATED1;
	  for(unsigned char i = 0; i<8; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++; 
	  }
   flag1 = 1;
   setappointmentdate();
  }
   if(flag1 == 1)
   flag2 = 1;
}

void checkKeyAppointmentTime()
{
  input = '\0';
  while ((input = keypad.getKey()) == 0);
  if(input == '#')
  {
	  int firstaddress = APPTIMEH1;
	  for(unsigned char i = 0; i<6; i++)
	  {
		EEPROM.write(firstaddress,0);
		firstaddress++;
		  
	  }
   flag1 = 1;
   setappointmenttime();
  }
   if(flag1 == 1)
   flag2 = 1;
}


void checkTime1Correctness()
{
  if(unpackedBCDtoInt(TIME1H1, TIME1H2)>=24 || unpackedBCDtoInt(TIME1M1, TIME1M2)>=60 || unpackedBCDtoInt(TIME1S1, TIME1S2)>=60)
  {
	lcd_clr();
	lcd_const_str_wrt("Timer1 time is");
	next_line();
	lcd_const_str_wrt("not correct");
	delay(1000);
  }
}

void checkTime2Correctness()
{
  if(unpackedBCDtoInt(TIME2H1, TIME2H2)>=24 || unpackedBCDtoInt(TIME2M1, TIME2M2)>=60 || unpackedBCDtoInt(TIME2S1, TIME2S2)>=60)
  {
  	lcd_clr();
  	lcd_const_str_wrt("Timer2 time is");
  	next_line();
  	lcd_const_str_wrt("not correct");
  	delay(1000);
  }
}

void checkAppointmentTimeCorrectness()
{
  if(unpackedBCDtoInt(APPTIMEH1, APPTIMEH2)>=24 || unpackedBCDtoInt(APPTIMEM1, APPTIMEM2)>=60 || unpackedBCDtoInt(APPTIMES1, APPTIMES2)>=60)
  {
  	lcd_clr();
  	lcd_const_str_wrt("App. time is");
  	next_line();
  	lcd_const_str_wrt("not correct");
  	delay(1000);
  }
}

void checkRTCTimeCorrectness()
{
  if(medicineTime.hour>=24 || medicineTime.min>=60 || medicineTime.sec>=60)
  {
  	lcd_clr();
  	lcd_const_str_wrt("RTC time is");
  	next_line();
  	lcd_const_str_wrt("not correct");
  	delay(1000);
  }
}

void checkRTCDateCorrectness()
{
  if(medicineTime.date>=32 || medicineTime.mon>=13 || medicineTime.year==0)
  {
	lcd_clr();
	lcd_const_str_wrt("RTC Date is");
	next_line();
	lcd_const_str_wrt("not correct");
	delay(1000);
  }
}

void indicationForAppointment()
{
	lcd_clr();
	lcd_const_str_wrt("Tomorrow you ");
	next_line();
	lcd_const_str_wrt("have an app.");
	delay(2000);
	lcd_clr();
	char arr[10];
	int firstaddress = APPTIMEH1;
	arr[0] = '@'; arr[1] = ' ';
	for(unsigned char i = 2; i <= 9; i++)
	{
		if(i == 4 || i == 7)
		{
			arr[i] = ':';
			continue;
		}
		arr[i] = EEPROM.read(firstaddress);
		firstaddress++;
	}
 lcd_clr();
 for(unsigned char i = 0; i <= 9; i++)
 {
    goto_location(1,i);
    lcd_data_wrt(arr[i]);
 }
	delay(2000);
  return;
}

void convertRTCTimeForLcd()
{
  unsigned char timer[8];
  unsigned char date[10];
  medicineTime = rtc.getTime();
  unsigned char day = medicineTime.date;
  unsigned char month = medicineTime.mon;
  unsigned int year = medicineTime.year;
  
  timer[2] = timer[5] = ':';
  timer[0] = 48 + (medicineTime.hour/10);
  timer[1] = 48 + (medicineTime.hour%10);
  timer[3] = 48 + (medicineTime.min/10);
  timer[4] = 48 + (medicineTime.min%10);
  timer[6] = 48 + (medicineTime.sec/10);
  timer[7] = 48 + (medicineTime.sec%10);
  date[2] = date[5] = '-';
  date[0] = 48 + (day/10);
  date[1] = 48 + (day%10);
  date[3] = 48 + (month/10);
  date[4] = 48 + (month%10);
  date[6] = 48 + (year/1000);
  date[7] = 48 + (year-((date[6]-48)*1000))/100;
  date[8] = 48 + (year-(((date[6]-48)*1000)+((date[7]-48)*100)))/10;
  date[9] = 48 + (year-(((date[6]-48)*1000)+((date[7]-48)*100)+((date[8]-48)*10)));
  lcd_clr();
  lcd_const_str_wrt("Time: ");
  goto_location(1,6);
  lcd_data_wrt(timer[0]);
  goto_location(1,7);
  lcd_data_wrt(timer[1]);
  goto_location(1,8);
  lcd_data_wrt(timer[2]);
  goto_location(1,9);
  lcd_data_wrt(timer[3]);
  goto_location(1,10);
  lcd_data_wrt(timer[4]);
  goto_location(1,11);
  lcd_data_wrt(timer[5]);
  goto_location(1,12);
  lcd_data_wrt(timer[6]);
  goto_location(1,13);
  lcd_data_wrt(timer[7]);
  next_line();
  lcd_const_str_wrt("Date: ");
  goto_location(2,6);
  lcd_data_wrt(date[0]);
  goto_location(2,7);
  lcd_data_wrt(date[1]);
  goto_location(2,8);
  lcd_data_wrt(date[2]);
  goto_location(2,9);
  lcd_data_wrt(date[3]);
  goto_location(2,10);
  lcd_data_wrt(date[4]);
  goto_location(2,11);
  lcd_data_wrt(date[5]);
  goto_location(2,12);
  lcd_data_wrt(date[6]);
  goto_location(2,13);
  lcd_data_wrt(date[7]);
  goto_location(2,14);
  lcd_data_wrt(date[8]);
  goto_location(2,15);
  lcd_data_wrt(date[9]);
  
  medicineTime = rtc.getTime();
  timer[6] = 48 + (medicineTime.sec/10);
  timer[7] = 48 + (medicineTime.sec%10);
  goto_location(1,12);
  lcd_data_wrt(timer[6]);
  goto_location(1,13);
  lcd_data_wrt(timer[7]);
  delay(1000);
  return;
}
