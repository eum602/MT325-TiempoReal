#include <16f877a.h>
#device ICD=TRUE
#fuses HS,NOWDT,NOPROTECT,NOLVP 
#use delay(clock=10000000) 
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8) 
#use FAST_IO(A) 
#use FAST_IO(B)
#use FAST_IO(C) 

// Definicion de variables
   char q;
// *********************

// Declaracion de Funciones
   
//*************************


// Funcion Principal
void main(){
   for(;;){
      q=getch();
      printf("%c\r",q);//q contiene el valor del convertidor ana / dig
      delay_ms(100);
   }
}
// Fin de la Funcion Principal


// Definicion de la Funciones
   
//***************************
