
char Manual[] = "\tHello and welcome to Anis & Polina RGB Calibration\n\nThe program once started will print out 15\nsampled Red,Green and blue light level.\nTo start the program all you have to do,\nInsert a separation page between the RGB LED\nand the photoresistor, user should wait until data\nIs displayed on communication port and no LEDs OFF";
char Error_Message[] = "ERROR ERROR ERROR, Consult the owners program is HALTED";

//Macros
//------------------------------------------------------------------------------------------------ 
#define Max(a, b) (a>b?a:b)
#define IsBigger(a, b) (a>b?1:0)
#define IsValid_INTpin(a) (a>1 && a<4 ?1:0)
//------------------------------------------------------------------------------------------------ 

//Definitions
//------------------------------------------------------------------------------------------------ 
#define Number_Of_Samples  3
//------------------------------------------------------------------------------------------------

//Enums & Flags
//------------------------------------------------------------------------------------------------
enum Program_Status{
  Idle,
  Start,
  Sampling,
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
int Red_LED           = 4 ;
int Blue_LED          = 5 ;
int Green_LED         = 6 ;
int Photo_Interrupter = 2 ; //KY-010 sensor will be connected to INTERRUPT 0 


int Arry_Sampled_LightLevel_ON [Number_Of_Samples];
int Arry_Sampled_LightLevel_OFF[Number_Of_Samples];
int Arry_Sampled_LightLevel_ActualLight_Red[Number_Of_Samples];
int Arry_Sampled_LightLevel_ActualLight_Green[Number_Of_Samples];
int Arry_Sampled_LightLevel_ActualLight_Blue[Number_Of_Samples];
//------------------------------------------------------------------------------------------------


void setup() {

  //************************* UART/Console Configure
  Serial.begin(9600);
  Serial.println(Manual);
  
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
    detachInterrupt(digitalPinToInterrupt(Photo_Interrupter)); //Turns off the given interrupt
    Serial.println("Sampling Mode started, when LED is white please move forward/set the sampling object");
    RGB_LED_OFF();
    for(int i = 0 ; i<3;i++){
      for(int J_Sample = 0 ; J_Sample < Number_Of_Samples;J_Sample++)
      {
        White_LED_On();
        My_Delay(6000);//delay 1 sec User must move the object away/set
        RGB_LED_OFF();
        My_Delay(750);
        int temp_Analog_value_off_state = analogRead(PhotoResistor);
        switch (i)
        {
          case 0: //RED
            Red_LED_On(); 
            break;
          case 1://Blue
            Blue_LED_On();
            break;
          case 2://Green
            Green_LED_On(); 
            break;
          default:
            Serial.println(Error_Message);
            while(1);
        } 
        My_Delay(750);
        int temp_Analog_value_on_state = analogRead(PhotoResistor);
        RGB_LED_OFF();
        int val = 0 ;
        //check which light is bigger, in other words an absolute value is stored
        if(IsBigger(temp_Analog_value_on_state,temp_Analog_value_off_state))
        {
          val = temp_Analog_value_on_state - temp_Analog_value_off_state; 
        }
        else{
          val = temp_Analog_value_off_state - temp_Analog_value_on_state;
          }
        switch (i){
          case 0: //RED
            Arry_Sampled_LightLevel_ActualLight_Red[J_Sample] = val; 
            break;
          case 1://Blue
            Arry_Sampled_LightLevel_ActualLight_Blue[J_Sample] = val; 
            break;
          case 2://Green
            Arry_Sampled_LightLevel_ActualLight_Green[J_Sample] = val; 
            break;
          default:
            Serial.println(Error_Message);
            while(1);
          }// !switch 
        }//!J_Smapling
      }//!All_colors for i loop
    RGB_LED_OFF();
    Display_All_Data();
    Program_Status_Flag = Display;
    Disable_TImer1();
    }
}




//Functions
//-----------------------------------------------------------------------------------------
//---------------------------------- User functions
void Display_All_Data(void){
  Serial.println("Printing all sampled data ");
  for(int i = 0 ; i<3;i++){
    switch (i){
      case 0: //RED
        Serial.println("Red Samples : "); 
        break;
      case 1://Blue
        Serial.println("Blue Samples : "); 
        break;
      case 2://Green
        Serial.println("Green Samples : ");
        break;
      default:
        Serial.println(Error_Message);
        while(1);
      }// !switch 
      
      for(int J_Sample = 0 ; J_Sample < Number_Of_Samples;J_Sample++)
      {
        switch (i){
          case 0: //RED
            Serial.println(Arry_Sampled_LightLevel_ActualLight_Red[J_Sample]); 
            break;
          case 1://Blue
            Serial.println(Arry_Sampled_LightLevel_ActualLight_Blue[J_Sample]); 
            break;
          case 2://Green
            Serial.println(Arry_Sampled_LightLevel_ActualLight_Green[J_Sample]); 
            break;
          default:
            Serial.println(Error_Message);
            while(1);
          }// !switch
      }// !J_Samples
      Serial.println("End off current LED sample");
  }
}
void My_Delay(int Time_In_ms){
  int count = Time_In_ms/250;
  int temp;
  Timer1_Status_Flag = Stop;
  Enable_TImer1();
  while(count){
    while(Timer1_Status_Flag == Stop){
      Serial.print("");
      }
    count = count-1;
    Timer1_Status_Flag = Stop;
    }
    Disable_TImer1();
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
