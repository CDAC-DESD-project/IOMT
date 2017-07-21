#define setbit(A,B) A|=(1<<B)
#define clrbit(A,B) A&=(~(1<<B))
#define togbit(A,B) A^=(1<<B)

#define RS 2
//#define RW 1 // RW Pin of LCD is connected to ground.
#define E 3

#define LCD_CURSERBLINKOFF 0x0C
#define LCD_DISPLAYON_CURSORBLINKING 0x0F

#define TIME1H1 0x00	// Timer1 EEPROM starting address
#define TIME1H2 0x01
#define TIME1M1 0x02
#define TIME1M2 0x03
#define TIME1S1 0x04
#define TIME1S2 0x05

#define TIME2H1 0x06	// Timer2 EEPROM starting address
#define TIME2H2 0x07
#define TIME2M1 0x08
#define TIME2M2 0x09
#define TIME2S1 0x0A
#define TIME2S2 0x0B


#define RTCTIMEH1 0x14	// RTC Timer EEPROM starting address
#define RTCTIMEH2 0x15
#define RTCTIMEM1 0x16
#define RTCTIMEM2 0x17
#define RTCTIMES1 0x18
#define RTCTIMES2 0x19

#define RTCDATED1 0x1A	// RTC Date EEPROM starting address
#define RTCDATED2 0x1B
#define RTCDATEM1 0x1C
#define RTCDATEM2 0x1D
#define RTCDATEY1 0x1E
#define RTCDATEY2 0x1F
#define RTCDATEY3 0x20
#define RTCDATEY4 0x21

#define APPDATED1 0x22	// Appointment Date EEPROM starting address
#define APPDATED2 0x23
#define APPDATEM1 0x24
#define APPDATEM2 0x25
#define APPDATEY1 0x26
#define APPDATEY2 0x27
#define APPDATEY3 0x28
#define APPDATEY4 0x29

#define APPTIMEH1 0x2A	// Appointment Time EEPROM starting address
#define APPTIMEH2 0x2B
#define APPTIMEM1 0x2C
#define APPTIMEM2 0x2D
#define APPTIMES1 0x2E
#define APPTIMES2 0x2F



void enterTime1();		// when first medicine time is entered
void enterTime2();		// when second medicine time is entered
void setrtctimer();		// when rtc time is entered
void setrtcdate();		// when rtc date is entered
void welcomeMessage();	// Startup message
void compare();			// comparing real date and time with saved time
void setappointmentdate();	// setting appointment date
void setappointmenttime();	// setting appointment time
void blinkingLedFirstMedicine();	// blinking the LED at first medicine time
void blinkingLedSecondMedicine();	// blinking the LED at second medicine time
void checkResetTimer1Time();		// checking the pressed key for timer1 module	
void checkResetTimer2Time();		// checking the pressed key for timer2 module
void checkKeyRTCTime();				// checking the pressed key for RTC time module
void checkKeyRTCDate();				// checking the pressed key for RTC date module
void checkKeyAppointmentDate();		// checking the pressed key for appointment date module
void checkKeyAppointmentTime();		// checking the pressed key for appointment time module
void checkTime1Correctness();		// checking the correctness of first medicine time
void checkTime2Correctness();		// checking the correctness of second medicine time
void checkAppointmentTimeCorrectness();		// checking the correctness of appointment time
void checkRTCTimeCorrectness();				// checking the correctness of RTC time.
void checkRTCDateCorrectness();				// checking the correctness of RTC date.
void indicationForAppointment();			// invoke the reminder and shows the appointment time.
unsigned int unpackedBCDtoInt(int, int);	// converting unpacked BCD to integer.
unsigned int unpackedyearBCDtoInt(int, int);		// converting unpacked BCD to integer.
void configMessage();						// Asking to save the entered data.
void introMessage();

 
void port_init(void);

// LCD Functions 
void lcd_init_4bit(void);
void lcd_data_wrt(unsigned char data);
void lcd_cmd_wrt(unsigned char data);
void cmd_wrt_en(void);
void data_wrt_en(void);
void lcd_const_str_wrt(char *str);
void lcd_str_wrt(unsigned char data[]);
void shift_disp_right(unsigned char times);
void shift_disp_left(unsigned char times);
void next_line(void);
void lcd_clr(void);
void goto_location(unsigned char x, unsigned char y);
void inttostr(unsigned int x, unsigned char str[]);
//