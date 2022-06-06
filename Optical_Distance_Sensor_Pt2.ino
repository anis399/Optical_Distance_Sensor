
char Manual[] = "\tHello and welcome to Anis & Polina RGB Calibration\n\nThe program once started will print out 15\nsampled Red,Green and blue light level.\nTo start the program all you have to do,\nInsert a separation page between the RGB LED\nand the photoresistor, user should wait until data\nIs displayed on communication port and no LEDs OFF";
char Error_Message[] = "ERROR ERROR ERROR, Consult the owners program is HALTED";

#define Mean_Buffer_Size  14
int Red_values[Mean_Buffer_Size]   = {150,130,111,93, 76, 63, 48, 41, 35, 34, 32, 29, 22, 19};
int Blue_values[Mean_Buffer_Size]  = {66 ,52 ,41 ,31, 26, 21, 16, 13, 12, 11, 9 , 8 , 6 , 5 };
int Green_values[Mean_Buffer_Size] = {78 ,63 ,48 ,40, 32, 26, 18, 17, 15, 12, 11, 10, 8 , 6 };

int Mean_AVG[Mean_Buffer_Size] = {0};

//Macros
//------------------------------------------------------------------------------------------------ 
#define Max(a, b) (a>b?a:b)
#define IsBigger(a, b) (a>b?1:0)
#define IsValid_INTpin(a) (a>1 && a<4 ?1:0)
//------------------------------------------------------------------------------------------------ 

//Definitions
//------------------------------------------------------------------------------------------------ 
#define Number_Of_Samples  4
//------------------------------------------------------------------------------------------------

//Enums & Flags
//------------------------------------------------------------------------------------------------
enum Program_Status{
  Idle,
  Start,
  Sampling,
  Busy,
  Display,
  Active,
  Stop
  };
  
enum LED_Status{
  Red,
  Green,
  Blue
  };
  
Program_Status Program_Status_Flag = Idle;
Program_Status Timer1_Status_Flag  = Stop;
LED_Status Program_Status_LED_Flag = Red;
//------------------------------------------------------------------------------------------------


//Variables
//------------------------------------------------------------------------------------------------
int PhotoResistor     = A0; 
//More light give us a lower analogRead value
//Less light gives us a higher analogRead value
int Photo_Interrupter = 2 ; //KY-010 sensor will be connected to INTERRUPT 0 
int Red_LED           = 4 ;
int Blue_LED          = 5 ;
int Green_LED         = 6 ;
int Buzzer            = 7 ;
//------------------------------------------------------------------------------------------------

void setup() {
  //************************* UART/Console Configure
  Serial.begin(9600);
  Serial.println(Manual);

  //************************* Calculate the avarge mean
  Calculate_MeanAVG();
  
  //************************* RGB LED Configure
  pinMode(Red_LED  , OUTPUT);
  pinMode(Blue_LED , OUTPUT);
  pinMode(Green_LED, OUTPUT);
  RGB_LED_OFF();
 
  //************************* Interrupts Configure
  INTERRUPT0_Config();
  TIMER1_Config(); //configured for interrupt every 250ms/ 4Hz
  
  //************************* Allow Interrupts
  Program_Status_Flag = Idle;
  Serial.println("Program has started");
  Disable_TImer1();
  sei();
 }

void loop() {
  if(Program_Status_Flag == Sampling){
    Program_Status_Flag = Busy;
    //detachInterrupt(digitalPinToInterrupt(Photo_Interrupter)); //Turns off the given interrupt
    Serial.println("Sampling Mode started, when LED is white please move forward/set the sampling object");
    RGB_LED_OFF();
    //for(int i = 0 ; i<3;i++){
    White_LED_On();
    My_Delay(6000);//delay 1 sec User must move the object away/set
    RGB_LED_OFF();
    My_Delay(250);
    int temp_Analog_value_off_state_red = analogRead(PhotoResistor);
    Red_LED_On();
    My_Delay(250);
    int temp_Analog_value_on_state_red = analogRead(PhotoResistor);

    RGB_LED_OFF();
    My_Delay(250);
    int temp_Analog_value_off_state_green = analogRead(PhotoResistor);
    Green_LED_On();
    My_Delay(250);
    int temp_Analog_value_on_state_green = analogRead(PhotoResistor);

    RGB_LED_OFF();
    My_Delay(250);
    int temp_Analog_value_off_state_blue = analogRead(PhotoResistor);
    Blue_LED_On();
    My_Delay(250);
    int temp_Analog_value_on_state_blue = analogRead(PhotoResistor);
    RGB_LED_OFF();
    
    int TrueRed = temp_Analog_value_off_state_red - temp_Analog_value_on_state_red;
    if(TrueRed < 0)
      TrueRed = -1 * TrueRed;
    int TrueGreen = temp_Analog_value_off_state_green - temp_Analog_value_on_state_green;
    if(TrueGreen < 0)
      TrueGreen = -1 * TrueGreen;
    int TrueBlue = temp_Analog_value_off_state_blue - temp_Analog_value_on_state_blue;
    if(TrueBlue < 0)
      TrueBlue = -1 * TrueBlue;

    Serial.print("True Red");
    Serial.println(TrueRed);
    Serial.print("True Blue");
    Serial.println(TrueBlue);
    Serial.print("True Green");
    Serial.println(TrueGreen);
    
    int Mean_AVG_Current_value = (TrueRed + TrueGreen + TrueBlue)/3;
    Serial.print("Mean AVG Current value: ");
    Serial.println(Mean_AVG_Current_value);
    
    int distance = Find_Distance(Mean_AVG_Current_value-20);

    for(int i = 0; i < distance;i++){
      tone(Buzzer, 1000); // Send 1KHz sound signal...
      My_Delay(500);        // ...for 1 sec
      noTone(Buzzer);     // Stop sound...
      My_Delay(500);
      }
    Program_Status_Flag = Display;
    Disable_TImer1();
    Program_Status_Flag = Idle;
    EIFR = (1 << 0);
    EIFR = (1 << 1);
    attachInterrupt(digitalPinToInterrupt(Photo_Interrupter),PhotoInterrupter_CallBack,HIGH);
    }
}


//Functions
//-----------------------------------------------------------------------------------------
//---------------------------------- User functions
int Find_Distance(int Mean_AVG_Current_value)
{
  for(int i = 0 ; i < Mean_Buffer_Size; i++){ 
    if(Mean_AVG[i] < Mean_AVG_Current_value){
      Serial.print("Index: ");
      Serial.print(i);
      Serial.print(" ");
      return i;
    }
  }
  Serial.println();
}

void Calculate_MeanAVG(void)
{
  Serial.println("Calculated Mean values: ");
  for(int i = 0 ; i < Mean_Buffer_Size; i++){
    int tempVal = Red_values[i] + Green_values[i] + Blue_values[i];
    Mean_AVG[i] = tempVal/3;
    Serial.print(Mean_AVG[i],DEC);
    Serial.print(" ");
    }
    Serial.println();
}

void My_Delay(int Time_In_ms){
  //Serial.print("My delay started  ");
  int count = Time_In_ms/250;
  int temp;
  //Serial.println(count);
  Timer1_Status_Flag = Stop;
  Enable_TImer1();
  while(count){
    while(Timer1_Status_Flag == Stop){
      //temp = temp + 1;//
      Serial.print("");
      }
    count = count-1;
    Timer1_Status_Flag = Stop;
    //Serial.println(count);
    }
    Disable_TImer1();
    //Serial.println("My delay finish");
  }
void Red_LED_On(void){
  digitalWrite(Red_LED,HIGH);
  }
void Green_LED_On(void){
  digitalWrite(Green_LED,HIGH);
  }
void Blue_LED_On(void){
  digitalWrite(Blue_LED,HIGH);
  }
void White_LED_On(void){
  digitalWrite(Red_LED,HIGH);
  digitalWrite(Blue_LED,HIGH);
  digitalWrite(Green_LED,HIGH);
  }
void RGB_LED_OFF(void){
  digitalWrite(Red_LED,LOW);
  digitalWrite(Blue_LED,LOW);
  digitalWrite(Green_LED,LOW);
  }
//---------------------------------- Interrupts Callbacks
ISR(TIMER1_COMPA_vect){
  Timer1_Status_Flag = Active;
  //Serial.println("Timer is Active");
  TCNT1 = 0;
  }
  
void PhotoInterrupter_CallBack(void){
  EIFR = (1 << 0);
  detachInterrupt(digitalPinToInterrupt(Photo_Interrupter)); //Turns off the given interrupt
  if(Program_Status_Flag == Idle)
    Program_Status_Flag = Sampling;
  Serial.println("PhotoInterrupter has triggered an interrupt");
 }
//---------------------------------- Interrupts Configure
void INTERRUPT0_Config(void){
  pinMode(Photo_Interrupter,INPUT_PULLUP); //setup pin as input_pullup
  if(IsValid_INTpin(Photo_Interrupter))
    attachInterrupt(digitalPinToInterrupt(Photo_Interrupter),PhotoInterrupter_CallBack,RISING);//LOW CHANGE RISING FALLING
  else{
    cli();//stop interrupts
    detachInterrupt(digitalPinToInterrupt(Photo_Interrupter)); //Turns off the given interrupt
    Serial.println("Interrupt test pin selected");
    Serial.println("/tphoto Interrupter pin selections is incorrect\nProgram halted");
    while(1);
    }
  }

void TIMER1_Config(void){
  cli();//stop interrupts
  //set timer1 interrupt at 4Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1 = 0;//initialize counter value to 0
  //set compare match register for 1hz increments
  OCR1A = 3905;// = (16*10^6) / (4*1024)- 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // disable timer compare interrupt
  Disable_TImer1();
  }

void Enable_TImer1(void){
  TCNT1 = 0;
  TIMSK1 |= (1 << OCIE1A);
  }
void Disable_TImer1(void){
  TCNT1 = 0;
  TIMSK1 &= ~(1 << OCIE1A);
  }
