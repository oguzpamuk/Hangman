

/***************************************************/
/*  Author: Oguzcan PAMUK                          */
/*  Course: CSE 334                                */
/*  Number: 111044053                              */
/***************************************************/ 

#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include <stdio.h>

/***************************************************/
/*                 Makrolar                        */  
/***************************************************/ 

#define LCD_DAT PORTK /*Port K drives LCD data pins, E, and LCD_RS */
#define LCD_DIR DDRK  /*Direction of LCD port */
#define LCD_E 0x02    /*LCD E signal */
#define LCD_RS 0x01   /*LCD Register Select signal */
#define CLEAR 0x01
#define LETTER_SIZE 26
/*Timer enable - disable*/
#define enable()__asm(cli)
#define disable()__asm(sei)
#define INTERRUPT_BUZZER interrupt (((0x10000 - Vtimch5)/2)-1)
#define INTERRUPT_DC interrupt (((0x10000 - Vtimch2)/2)-1)


#define DELAY_BUZZER 12
#define LOSE_GAME_BUZZER 1915

/* oyunu kazaninca notalara uygun buzzer cal*/
/* yanlis bilince notasýz da olsa buzzer caliyor*/
#define A_NOTE 1136
#define B_NOTE 1014
#define C_NOTE 1915 
#define D_NOTE 1700
#define E_NOTE 1519 
#define F_NOTE 1432
#define G_NOTE 1275
#define b_NOTE 1073


/********score********/
#define PLUS 6
#define MINUS 1

#define TRUE 1 
#define FALSE 0 

/***************************************************/
/*          Fonksiyon Prototipleri                 */  
/***************************************************/ 
  
void openlcd(void);
void puts2lcd(char str[]); 
void put2lcdData(unsigned char c);
void put2lcdCommand(unsigned char c);
void Delay(unsigned short num);
void startGame(void);
unsigned char whichLetter(unsigned char asci);
INTERRUPT_BUZZER void soundBuzzer(void);
void initInterrupt(void);
void enableBuzzer(void);
void disableBuzzer(void);
unsigned char getFromKeypad(void);
unsigned char howManyLetters(const unsigned char word[],unsigned char letter);
void endGame(int loseOrWin);
void calculateScore(unsigned char bildigi,unsigned char bilemedigi);
void input_lcd(void);
INTERRUPT_DC void led(void);
char read(void);
void write(char letterWrite);
void setupSCI(void); 
unsigned char str_size(const unsigned char word[]);
void index_doldur(unsigned char letter,unsigned char word[]);
void show_value(void);
void writeStringToConsole(const unsigned char word[]);
unsigned char enterKeyPad(void);
int ReadKey();

/***************************************************/
/*          Global Degiskenler                     */  
/***************************************************/

/*Keypad icin tanimli*/
const unsigned char letterAsci[]={65,66,67,68,69,70,71,72,73,74,75,76,77,78,
                                  79,80,81,82,83,84,85,86,87,88,89,90};

const unsigned char letters[]={'A','B','C','D','E','F','G','H','I','J','K','L',
                               'M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

/* level 1 100 tane level 2 100 tane toplam 200 word */
const unsigned char level1_words[][6]={"ABLE","BOOK","DEEP","DESK",
                                       "DIRT","GAME","GIRL","GOOD",
                                       "GREY","HARD","HATE","HEAR",
                                       "HIGH","HILL","HOLD","HOPE",
                                       "HOUR","HURT","IDEA","KNOW",
                                       "INTO","IRON","JOKE","KNEE",
                                       "LAMB","LEFT","LIFE","LION",
                                       "LOSS","LOVE","LUCK","LIVE",
                                       "LOOK","ACID","ALSO","AREA",
                                       "ARMY","ATOM","AWAY","BABY",
                                       "BACK","BAND","BANK","BEAN",
                                       "BEAK","BEAM","BEAR","BEAT",
                                       "BELT","BEND","BEST","BILL",
                                       "BITE","BLUE","BODY","BOMB",
                                       "BOOT","BOTH","BOWL","CARD",
                                       "CASE","CELL","CHIN","CITY",
                                       "COIN","COLD","COPY","COST",
                                       "CROP","LEFT","DATE","DEAL",
                                       "DESK","DIRT","DUST","EAST",
                                       "ELSE","EVEN","EVER","FACE",
                                       "FAIR","FARM","FAST","FEAR",
                                       "FILM","FIND","FIRM","FISH",
                                       "FIVE","FLAG","FOOD","FLOW",
                                       "GATE","GLAD","GAIN","GOAT",
                                       "LINE","LIST","LIFT","LEND",
                                       "LEARN","CURVE","LEVEL","LIMIT",
                                       "LITRE","LOWER","LUCKY","ABOUT",
                                       "ABOVE","AGAIN","AGREE","AHEAD",
                                       "ALLOW","ALONE","ALONG","ANGLE",
                                       "ANGRY","APART","APPLE","BASIC",
                                       "BEGIN","BELOW","BLOCK","BLACK",
                                       "BLIND","BORED","BRAIN","BREAK",
                                       "BREED","BRING","BROWN","CATCH",
                                       "CAUSE","CHAIR","CHASE","CHEAP",
                                       "CHEEK","CHIEF","CLOTH","COAST",
                                       "COULD","COUNT","CRIME","CRUEL",
                                       "CYCLE","DAILY","DEATH","DECAY",
                                       "DELAY","DEPTH","ADMIT","DIRTY",
                                       "DREAM","EAGER","EARLT","ELBOW",
                                       "EMPTY","ENJOY","EQUAL","EVENT",
                                       "EVERY","EXACT","EXIST","FEVER",
                                       "FINAL","FIRST","FLOAT","GLASS",
                                       "GRAND","GRAVE","GREAT","GUARD",
                                       "GUIDE","HABIT","HAPPY","HEART",
                                       "HORSE","HOTEL","HUMAN","HURRY",
                                       "INNER","JOINT","KLICK","SKILL",
                                       "KNEEL","KNOCK","LAUGH","LAYER",
                                       "LEAST","LEAVE","LEGAL","LEVEL",
                                       "LOCAL","GUESS","GARIN","FRESH",
                                       "FILET","ENTER","DRUNK","DOUBT",}; 

/*kazaninca calacak sarkinin notalarini tutuyor*/
const unsigned int winn_game[] = { E_NOTE, E_NOTE, E_NOTE, C_NOTE, E_NOTE, G_NOTE, G_NOTE, 
                                   C_NOTE, G_NOTE, E_NOTE, A_NOTE, B_NOTE, b_NOTE, A_NOTE, 
                                   G_NOTE, E_NOTE, G_NOTE, A_NOTE, F_NOTE, G_NOTE, E_NOTE, 
                                   C_NOTE, D_NOTE, B_NOTE };

const unsigned int dc_high[]= {3000,7000};
const unsigned int dc_low[]= {2000,8000};

/*notalar arasi gecis icin*/                                  
volatile unsigned char buzzerCounter=0;

/*kelimeyi ekranda gostermek icin*/
unsigned char value[]={'-','-','-','-','-'};
                                       
/* while dongusu sarti*/
volatile unsigned int done;

/*Kazandiysa 1 kaybettiyse 0*/
int loseOrWin=0;

/*Kazandigi puan*/
int score=0;

/*interrupt delay counter*/
volatile int delay=0;   

/*yeni oyun baslamali mi*/
unsigned char new_game=1;

/*oyunun temel degiskenleri*/
unsigned char bildigi=0,denemeHakki=0,letter,asci,temp,wordCounter=0;
/*bildigi sayi kadar tut*/
volatile unsigned char count=0,quit='c';
/*led parlakligi ve random olusturma*/
volatile unsigned int dc_counter=0,randN=0,level=1;

void main(void) {

    /*gerekli ayarlar set ediliyor*/
    done=FALSE;
    openlcd();
    DDRP=DDRP |0x04;
    startGame();
    initInterrupt();
    setupSCI();
    
  
    while(!done){
    
        /*yeni oyunsa lcd de ona gore birseyler basiliyor
          ve random yeni kelime uretiliyor*/
        if(new_game==1){
          
            puts2lcd("New Game"); 
            Delay(2000);
            put2lcdCommand(CLEAR);
            Delay(10);
            new_game=0;
            Delay(350);
            
            if(loseOrWin==0 && randN>99)
                wordCounter=randN-99;
            else if(loseOrWin==1 && randN<100)
                wordCounter=randN+100;
            else
                wordCounter=randN;
            
            loseOrWin=0;
        }
        
        /*input girmeye gerek var mý*/
        if((denemeHakki!=6) && (bildigi!=str_size(level1_words[wordCounter]))){
          
            input_lcd();

            writeStringToConsole("Keypad icin k giriniz \r\n");
            asci=read();
            
            if(asci=='k'){
                writeStringToConsole("Keypad Secildi\r\n");
                temp=enterKeyPad();
                letter=temp*10; 
                letter+=enterKeyPad();
                
                asci=whichLetter(letter);
                count=howManyLetters(level1_words[wordCounter],asci);
              
            }else{
                writeStringToConsole("Console Secildi\r\n");
                writeStringToConsole("Input Giriniz \r\n");
            
                asci=read();
                put2lcdCommand(CLEAR);
                Delay(10);
                put2lcdData(asci);
                Delay(1000);
                write(asci);
                write('\r');
                write('\n');
                put2lcdCommand(CLEAR);
                Delay(10);
                
                count=howManyLetters(level1_words[wordCounter],asci);    
            
            }
 
            
            if(count==0){
              enableBuzzer();
              disableBuzzer();
              Delay(500);
              denemeHakki=denemeHakki+1;
            }else{
              bildigi+=count;
              index_doldur(asci,level1_words[wordCounter]);
            }
            
        }else{
            
            writeStringToConsole("Word -->");
            writeStringToConsole(level1_words[wordCounter]);
            writeStringToConsole("\r\n");
            if(bildigi==str_size(level1_words[wordCounter])){
              
              loseOrWin=1;
              writeStringToConsole("Winn !! \r\n");
              enableBuzzer();
              disableBuzzer();
              level=2;
            }
            else{
              loseOrWin=0;
              writeStringToConsole("Loss !! \r\n");
              enableBuzzer();
              disableBuzzer(); 
              level=1;
            }
            
            calculateScore(bildigi,denemeHakki);
            endGame(loseOrWin);
            denemeHakki=0;
            bildigi=0;
            new_game=1;
            
            puts2lcd("Word-->");
            puts2lcd(level1_words[wordCounter]);
            Delay(2000);
            put2lcdCommand(CLEAR);
            Delay(10);  
            wordCounter+=1;
            value[0]='-'; 
            value[1]='-';
            value[2]='-';
            value[3]='-';  
            value[4]='-'; 
            
        }
        
    
    }   
                       
}

/***************************************************/
/*                  Fonksiyonlar                   */  
/***************************************************/ 

/*
    Gerekli interruptlar set ediliyor
*/


void initInterrupt(void){
     __asm BSET PUCR,1; // enable pull-up resistors on port A (dragon12-plus)
    /*timer ayarlari*/
    disable();
    
    /*output compare*/
    TSCR1 = 0x80;
    TIOS = TIOS | 0x04;          
    TCTL2 = (TCTL2 | 0x10) & ~0x20;
    TFLG1 = 0x04;      
    TIE = TIE | 0x04;
     
    
    enable();
  
}

/*
    Oyunun baslangici icin lcd'de yazilacaklar tutulur
*/
void startGame(void){

    put2lcdCommand(CLEAR);
    Delay(10);
    puts2lcd("Welcome to Game"); 
    Delay(2000);
    put2lcdCommand(CLEAR);
    Delay(10);
    puts2lcd("Are you ready !!");
    Delay(2000);
    put2lcdCommand(CLEAR);
    Delay(10);
    puts2lcd("Loading..."); 
    Delay(500);
    put2lcdCommand(CLEAR);
    Delay(10);
  
}

/*
    Bilinen harfleri lcd'de gosterilecek degiskene atar
*/

void index_doldur(unsigned char letter,unsigned char word[]){
    
    volatile unsigned char i=0;
    
    for(i=0 ; word[i] != '\0' ; i++){
       
        if(word[i]==letter)
            value[i]=letter;
    } 
  
}

/*
    Kelimenin bilinen kadarini lcd'de gostermeye yarar
*/

void show_value(void){
    put2lcdCommand(CLEAR);
    Delay(10);
    put2lcdData(value[0]);
    Delay(5);
    put2lcdData(value[1]);
    Delay(5);
    put2lcdData(value[2]);
    Delay(5);
    put2lcdData(value[3]);
    Delay(5);
    if(str_size(level1_words[wordCounter])==5){
        put2lcdData(value[4]);  
    }
    Delay(1000);
     
}

/*
    LCD'de input giriniz yazar
*/

void input_lcd(void){
  
    put2lcdCommand(CLEAR);
    Delay(10);
    puts2lcd("Input Giriniz..");
    Delay(500);
    put2lcdCommand(0xC0);
    Delay(1);
    show_value(); 
  
}

/*
    String'in size'ini verir
*/

unsigned char str_size(const unsigned char word[]){
  
    volatile unsigned char i=0;
    
    for(i=0 ; word[i] != '\0' ; i++){
       
    }
    
    return i; 
  
}
 

/*
    Oyun sonlandigi zaman yapilacaklar
*/

void endGame(int loseOrWin){
  
    char str[60];
    put2lcdCommand(CLEAR);
    
    if(loseOrWin==0){  
      puts2lcd("Game Over !!"); 
    }else{
      puts2lcd("Winner !!"); 
    }
    
    Delay(1000); 
    put2lcdCommand(CLEAR);
    sprintf(str,"SCORE --> %d",score);
    puts2lcd(str);
    Delay(1000); 
    put2lcdCommand(CLEAR);
      
}

/*
    Oyuncunun oyun sonu puanini hesaplar
*/

void calculateScore(unsigned char bildigi,unsigned char bilemedigi){
    
    if(bildigi!=0)
      score=(int) ((bildigi * PLUS) - (bilemedigi * MINUS));
    else
      score=0;
     
}

/* 1ms * num kadar delay uretir*/
void Delay(unsigned short num)
{
    volatile unsigned short i;
 
    
    while (num > 0){
        i = 2000;            
        while (i > 0){ 
            i = i - 1; 
        }                    
        num = num - 1;
    }
}


/*
  LCD init fonksiyonu
  LCD acilmasi icin gereken optimizasyonlari saglar
*/

void openlcd(void)
{
    LCD_DIR = 0xFF;
    Delay(1);
    put2lcdCommand(0x33);
    Delay(1);
    put2lcdCommand(0x32);
    Delay(1);   
    put2lcdCommand(0x28);   
    Delay(1);
    put2lcdCommand(0x0F);   
    Delay(1);
    put2lcdCommand(0x06);   
    Delay(1);
    put2lcdCommand(0x01);
    Delay(10);                 
}


/*
    Lcd'ye data gonderebilmek icin
*/

void put2lcdData(unsigned char c)
{

    unsigned char c_lo,c_hi;
      
    c_hi = (c & 0xF0) >> 2;
    LCD_DAT =  LCD_DAT & ~0x3C;                     
    LCD_DAT = LCD_DAT | c_hi;
    Delay(1);
    LCD_DAT = LCD_DAT | LCD_RS;
    Delay(1);
    LCD_DAT = LCD_DAT | LCD_E;
    Delay(1);
    LCD_DAT = LCD_DAT & ~LCD_E;
    Delay(5);
   
    c_lo = (c & 0x0F)<< 2;
    LCD_DAT =LCD_DAT & ~0x3C;                     
    LCD_DAT = LCD_DAT | c_lo;
    LCD_DAT = LCD_DAT | LCD_E;
    Delay(1);
    LCD_DAT = LCD_DAT & ~LCD_E;
    Delay(15); 
    
}

/*
    Lcd'ye komut gonderir
*/

void put2lcdCommand(unsigned char c){
  
        
    unsigned char c_lo,c_hi;
    
    c_hi = (c & 0xF0) >> 2;         
    LCD_DAT =LCD_DAT & ~0x3C;         
    LCD_DAT = LCD_DAT | c_hi;          
    Delay(1);
    LCD_DAT = LCD_DAT & ~LCD_RS;         
    Delay(1);
    LCD_DAT = LCD_DAT | LCD_E;          
    Delay(5);
    LCD_DAT = LCD_DAT & ~LCD_E;         
    Delay(15);
                           
    c_lo = (c & 0x0F)<< 2;          
    LCD_DAT =LCD_DAT & ~0x3C;        
    LCD_DAT =LCD_DAT | c_lo;             
    LCD_DAT = LCD_DAT | LCD_E;          
    Delay(5);
    LCD_DAT = LCD_DAT & ~LCD_E;         
    Delay(15);
  
}

/*
    Lcd'ye data gonderme fonksiyonunu kullanarak
    string gondermeye yarar
*/
void puts2lcd(char str[]) {

    int i=0;
  
    for(i=0 ; str[i] != '\0' ; i++){
  
        put2lcdData(str[i]);
        Delay(1);  
    }  


}

 

/* 
   Asci kodu verilen harfi dondurur
*/
unsigned char whichLetter(unsigned char asci){
     
     volatile unsigned char i;
     
     for(i=0;i<LETTER_SIZE;i++){
        
        if(asci==letterAsci[i])
          return letters[i];
     }
     
     return -1;
  
}

/* 
  Buzzer'i calistirir
  Kanal 5 i kullaniyorum
  Interrupt'i enable ediyorum
*/

void enableBuzzer(void){
                  
    TIOS = TIOS | 0x20;          
    TCTL1 = (TCTL1 | 0x04) & ~0x08; 
    TFLG1 = 0x20;     
    TIE = TIE | 0x20;
    
    if(loseOrWin==0){
      
        for(buzzerCounter=0;buzzerCounter<DELAY_BUZZER;buzzerCounter++){
        
            Delay(200); 
        }
    }
    
    if(loseOrWin==1){
      
        for(buzzerCounter=0;buzzerCounter<DELAY_BUZZER;buzzerCounter++){
        
            Delay(240); 
        }
    }
    
    
}

/* 
  Buzzer'i durdurur
  Tum ayarlari sifirlayarak durduruyorum
*/

void disableBuzzer(void){
  
    TCTL1 = 0x00;
  
}

/*
    Oyunun durumuna gore buzzer icin frekans olusturu
    output compare
*/

INTERRUPT_BUZZER void soundBuzzer(void){
  
    if(loseOrWin==0){
       TC5+=(LOSE_GAME_BUZZER * 2);
    }
    
    if(loseOrWin==1){
       TC5+=(winn_game[buzzerCounter] * 2);
    }
    
    TFLG1=0x20; 
}

/*
    Keypad'den data ceker
*/

unsigned char getFromKeypad(void){

	  const unsigned char keypad[][4]={'1','2','3','A',
	                                   '4','5','6','B',
	                                   '7','8','9','C',
	                                   '*','0','#','D'};
	  unsigned char row;
	  unsigned int col;

	  DDRA = 0xF0;

	  for(col=0;col<4;col++){
		  PORTA=~(0x10 << col);
		  row = PORTA & 0x0F;
		
		  switch(row){

			  case 0x0E:
				  return keypad[0][col];
			
			  case 0x0D:
			  	return keypad[1][col];
			
			  case 0x0B:
				  return keypad[2][col];

			  case 0x07:
				  return keypad[3][col];
		  }
	  }

	  return -1;
}

/*
    Kac harf bilindigini return eder
*/
unsigned char howManyLetters(const unsigned char word[],unsigned char letter){
 
    volatile unsigned char i=0;
    unsigned char countLetter=0;
    
    for(i=0 ; word[i] != '\0' ; i++){
       
        if(word[i]==letter){
          countLetter++;
        }
    }
    
    return countLetter;  
    
}

/*
    Led parlakligi icin
*/
INTERRUPT_DC void led(void){
    
    randN=randN+1;
    
    delay=delay+1;
    
    if(delay<500){
      
        if(dc_counter==0){
          
            TC2+=dc_high[0];
            dc_counter=1;
        }
        else{
          
            TC2+=dc_high[1];
            dc_counter=0;   
        }
                 
    }else if(delay>1000 && delay<1500){
         
        if(dc_counter==0){
          
            TC2+=dc_low[0];
            dc_counter=1;
        }
        else{
          
            TC2+=dc_low[1];
            dc_counter=0;   
        } 
      
    }else
        delay=0;
    
    
    
    if(randN==200)
      randN=0;     
   
    
    dc_counter=0;
  
    TFLG1 = 0x04;      
}

/*
    Console'dan karakter okur
*/
char read(void){
    
    while((SCI0SR1 & 0x20)== 0);
    
    return SCI0DRL;  
}

/*
    Console'a karakter yazar
*/
void write(char letterWrite){
  
    while((SCI0SR1 & 0x80)== 0);
    
    SCI0DRL=letterWrite;
    
}
/*
    Console init fonksiyonu
*/
void setupSCI(void){
  
    SCI0BDL = 156;
    SCI0BDH = 0;
    SCI0CR1 = 0x00;
    SCI0CR2 = 0x08;
    SCI0CR2 = 0x0C;
  
}

/*
    Console'a string basmaya yariyor
*/
void writeStringToConsole(const unsigned char word[]){
  
    int i=0;
  
    for(i=0 ; word[i] != '\0' ; i++){
  
        write(word[i]);
        Delay(1);  
    }  
  
}

unsigned char enterKeyPad(void){

    unsigned char control;
    Delay(50);
    while( (control = getFromKeypad()) == -1 );
    while( getFromKeypad() != -1);
    Delay(10);
    
    return control;
}





              
              
        
        
        
        