#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

volatile long light;
char result[100];
volatile long sample[100];
void ConfigureAdc_light();
void uart_init(void);
void ConfigClocks(void);
void strreverse(char* begin, char* end);
void itoa(int value, char* str, int base);
void port_init();

volatile int temp[50];
volatile int diff[50];
volatile unsigned int i=0;
volatile unsigned int j=0;
char th_char[5];
char tl_char[5];
char hh_char[5];
char hl_char[5];
volatile int hh = 0;
volatile int hl = 0;
volatile int th = 0;
volatile int tl = 0;
volatile int check = 0;
volatile int checksum = 0;
volatile int dataok;
char temperature[] = "Temperature is: ";
char dot[] = ".";
char celcius[] = " degrees Celcius  ";
char humidity[] = "Humidity is: ";
char percent[] = " %\r\n";

char light1[] = "Light is: ";
char lux[] = " lux  ";

void ser_output(char *str);
void main(void)
{
     WDTCTL = WDTPW | WDTHOLD;

     //port_init();
     //ConfigClocks();
     //uart_init();
     //_delay_cycles(5);
     //int i = 0;
     int acount = 0;

     BCSCTL1= CALBC1_1MHZ;
     DCOCTL = CALDCO_1MHZ;
     P1SEL = BIT1|BIT2;
     P1SEL2 = BIT1|BIT2;
     UCA0CTL1 |= UCSWRST+UCSSEL_2;
     UCA0BR0 = 52;
     UCA0BR1 = 0;
     UCA0MCTL = UCBRS_0;
     UCA0CTL1 &= ~UCSWRST;
     __delay_cycles(2000000);
        P2DIR |= BIT4;
        P2OUT &= ~BIT4;
        __delay_cycles(20000);
        P2OUT |= BIT4;
        __delay_cycles(20);
        P2DIR &= ~BIT4;
        P2SEL |= BIT4;
        TA1CTL = TASSEL_2|MC_2 ;
        TA1CCTL2 = CAP | CCIE | CCIS_0 | CM_2 | SCS ;
        _enable_interrupts();
        while (1){

            if (i>=40){

                ConfigureAdc_light();
                ADC10CTL0 |= ENC + ADC10SC + MSC;
                while((ADC10CTL0 & ADC10IFG) == 0);
                P1OUT |= BIT6;
                _delay_cycles(200000);
                light = ADC10MEM;
              // ltoa((light),result,10);
                P1OUT &= ~BIT6;                        // green LED off
                ADC10CTL0 &= ~ADC10IFG;

                acount = 0;

                                                            while(result[acount]!='\0')
                                                          {
                                                              while((IFG2 & UCA0TXIFG)==0);                  //Wait Unitl the UART transmitter is ready
                                                              UCA0TXBUF = result[acount++] ;                   //Transmit the received data.

                                                          }

                for (j = 1; j <= 8; j++){
                    if (diff[j] >= 110)
                        hh |= (0x01 << (8-j));
                    }
                for (j = 9; j <= 16; j++){
                    if (diff[j] >= 110)
                        hl |= (0x01 << (16-j));
                }
                for (j = 17; j <= 24; j++){
                    if (diff[j] >= 110)
                        th |= (0x01 << (24-j));
                }
                for (j = 25; j <= 32; j++){
                    if (diff[j] >= 110)
                        tl |= (0x01 << (32-j));
                }
                for (j = 33; j<=40; j++){
                    if (diff[j] >= 110)
                        checksum |= (0x01 << (40-j));
                }
                check=hh+hl+th+tl;
                if (check == checksum)
                    dataok = 1;
                else
                    dataok = 0;
                ltoa(th,th_char,10);
                ltoa(tl,tl_char,10);
                ltoa(hh,hh_char,10);
                ltoa(hl,hl_char,10);
                ltoa((light),result,10);
                ser_output(temperature); ser_output(th_char); ser_output(dot); ser_output(tl_char); ser_output(celcius);
                ser_output(light1); ser_output(result); ser_output(lux);
                ser_output(humidity); ser_output(hh_char); ser_output(dot); ser_output(hl_char); ser_output(percent);
                __delay_cycles(1000000);
                WDTCTL = WDT_MRST_0_064;
                }


        }
}
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A1(void){
        temp[i] = TA1CCR2;
        i += 1;
        TA1CCTL2 &= ~CCIFG ;
        if (i>=2) diff[i-1]=temp[i-1]-temp[i-2];
}
void ser_output(char *str){
    while(*str != 0){
        while (!(IFG2&UCA0TXIFG));
        UCA0TXBUF = *str++;
    }
}



void ConfigureAdc_light(){

    ADC10CTL0 &= ~ENC;
    ADC10CTL1 = INCH_4 + ADC10DIV_0 + CONSEQ_2;
    ADC10CTL0 = SREF_0 | ADC10SHT_3 | ADC10ON;
    __delay_cycles(5);
    ADC10CTL0 |= ENC| MSC;

}
