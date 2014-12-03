#include <18F4550.h>

#use delay(clock=10000000)
#use rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7,bits=8,parity=N)
#fuses HS,NOPROTECT,NOWDT,NOLVP,CPUDIV1,nomclr
#define Tiempo 200

#use FAST_IO(A) 
#use FAST_IO(B)
#use FAST_IO(D)

#byte T0CON=0xFD5
#byte wreg=0xFE8
#byte TOSU=0xFFF
#byte TOSH=0xFFE
#byte TOSL=0xFFD

int np=0;
int colap[6];
int m,m1;
int8 i,j,j1,j2,j3,j4;

int c1,c2;
int k1,k2,k3,k4,k5,k6;
int l1,l2,l3,l4,l5,l6;

char proceso;

int const lenbuff=10;
int valor2=0;
int32 val1,val2,val3,val4,val5;
//int16 quantum=65048;


int   xbuff=0x00;
char  cbuff[lenbuff];
char  rcvchar=0x00;
char  comando;
int1  flagcmd=0;


struct pcb{
int32 di;
int32 df; 
int32 da;
int estado; 
int wreg; 
int status;
int bsr;
int id;
int prioridad;
} vpcb[10];

// ======== VARIABLES DEL CALENDARIO ========== \\
// estado_calendario = 0 -->   No ejecutar 
// estado_calendario = 1 -->   Ejecutar 
// estado_calendario = 2 -->   Vacio
// estado_calendario = 3 -->   Cargando

int8 calendario[1000];
int reloj = 0;
int estado_calendario = 0;
int prioridad_act = 0;
//=========================
int16 mcm=0;
//=============================


void activar_timer()
{
   // set_timer0(65478);    //    para 3ms
   // set_timer0(65438);    //    para 5ms
   // set_timer0(65380);    //    para 8ms
   // set_timer0(65341);    //    para 10ms
   // set_timer0(65282);    //    para 13ms
   // set_timer0(65243);    //    para 15ms
   // set_timer0(65185);    //    para 18ms
   // set_timer0(65146);    //    para 20ms
    set_timer0(65048);    //    para 25ms
   // set_timer0(64950);    //    para 30ms
   //set_timer0(quantum);
   enable_interrupts(INT_TIMER0);
}

void desactivar_timer()
{
   disable_interrupts(INT_TIMER0);
}

void addcbuff(char c);
void mostrar_procesos();
void descargar_proceso(int idp);
void Config_TMR0();


#INT_TIMER0
void inter_quantum()
{  int i=0;
   desactivar_timer();
   output_toggle(PIN_D6);
   // Guardando variables
   vpcb[colap[0]].wreg=wreg;
   vpcb[colap[0]].da=(int32)TOSL;
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSH<<8);
   vpcb[colap[0]].da=vpcb[colap[0]].da|((int32)TOSU<<16);
   
   // corriendo proceso
   if(np>1)
   {   
      prioridad_act = 0;
      for (i=1; i<np; i++ ){
         if (vpcb[colap[i]].prioridad > prioridad_act){
            m1 = i;
            prioridad_act = vpcb[colap[i]].prioridad;
         }
      }
      
      m=colap[0];
      colap[0] = colap[m1];
      for(i=m1;i<np-1;i++)
      {
         colap[i]=colap[i+1];
      }
      colap[np-1]=m;
   }
   
   if(estado_calendario == 1){
      reloj++;
      if(reloj == mcm){
         reloj = 0;
      }
   }
   
   //==== PC=S0_Init() =======
   TOSL=00;
   TOSH=40;
   TOSU=00;
} 

void inte_Init()  // interrupcion de puerto serial
{
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_RDA);
}


#INT_RDA
void serial_isr()
{
   //printf("Recibido");     // se envia por Serial una palabra para verificar interrupcion
   rcvchar = 0x00;
   rcvchar = getc();    // se recibe el caracter enviado por la computadora
   addcbuff(rcvchar);   // guarda el caracter en un buffer
}

void addcbuff(char c)   
{
   switch(c)
   {
      case '*':                  // en caso de recibir *, levanta el flag para saber que se termino de enviar un comando.
         cbuff[xbuff++] = c;
         flagcmd = 1;
      break;
      default:
         cbuff[xbuff++] = c;
   }
}

void inicbuff(void)           // inicializa el buffer, limpiandolo
{
   int i;
   for (i=0; i<lenbuff;i++)      
   {
      cbuff[i] = 0x00;
   }
   xbuff=0x00;
}


void procesa_cmd(void){             // procesa el ultimo comando guardado, un comando esta comprendido entre $ hasta * 
 
   int i, j=0;                      
   char salida[lenbuff];            // Argumento de comando (si lo tiene)
   disable_interrupts(int_rda);     
   flagcmd=0;                       // Desactivo flag de comando pendiente.
   for (i=0;i<xbuff;i++)
   {
      if (cbuff[i] == '$'){         // verifica que tenga '$'
         while((cbuff[i] != '*') & (i<xbuff))   // hasta que encuentre *
         {
            salida[j++]=cbuff[i++];
         }
         comando = salida[1];
         valor2 = salida[2] -'0';
         val1 = salida[2];
         val2 = salida[3];
         
      }
   }
   inicbuff();                            // Borro buffer.
   for(i=0;i<lenbuff;i++)                 // Bucle que pone a 0 todos los
   {
      salida[i]=0x00;                     // caracteres en el argumento
   }
   enable_interrupts(int_rda);            
}


//*****************************************************************************
//PROGRAMA 1: Parpadeo Led D0

#ORG 0x1000, 0x1044
void Parpadear_D0()
{
   while(TRUE)
   {
      output_toggle(PIN_D0);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_D0);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
   }
}

//*****************************************************************************
//PROGRAMA 2: Parpadeo Led D1

#ORG 0x1046, 0x1090       
void Parpadear_D1()
{
   while(TRUE)
   {
      output_toggle(PIN_D1);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_D1);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
   }
}

//*****************************************************************************
//PROGRAMA 3: Parpadeo Led D2

#ORG 0x1092, 0x10D6
void Parpadear_D2()
{
   while(TRUE)
   {
      output_toggle(PIN_D2);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_D2);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
   }
}

//*****************************************************************************
//PROGRAMA 4: Parpadeo Led D3 - FINITO

#ORG 0x10D8, 0x1128
void Parpadear_D3()
{
   while(TRUE)
   {
      output_toggle(PIN_D3);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_D3);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      comando = 'D';
      valor2 = 4;
   }
}

//*****************************************************************************
//PROGRAMA 5: Parpadeo Led D4 - FINITO

#ORG 0x112A, 0x117A
void Parpadear_D4()
{
   while(TRUE)
   {
      output_toggle(PIN_D4);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_D4);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      comando = 'D';
      valor2 = 5;
   }
}


//*****************************************************************************
//PROGRAMA 6: Parpadeo Led B0 - FINITO

#ORG 0x117C, 0x11CC
void Parpadear_B0()
{
   while(TRUE)
   {
      output_toggle(PIN_B0);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_B0);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      comando = 'D';
      valor2 = 6;
   }
}

//*****************************************************************************
//PROGRAMA 7: Parpadeo Led B1 - FINITO
#ORG 0x11CE, 0x121E
void Parpadear_B1()
{
   while(TRUE)
   {
      output_toggle(PIN_B1);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      output_toggle(PIN_B1);
      for (k1=1;k1<170;++k1){for (l1=1;l1<170;++l1){}}
      comando = 'D';
      valor2 = 7;
   }
}


/*
// PROGRAMA 5: Mostrar Procesos

#ORG 0x2502, 0x2850           // COMANDO: $C4*
void task_manager(){
   while(TRUE){
      desactivar_timer();
      int m;
         int32 mproceso, mdirini, mdirfin, mdiract;
         printf("|   Proceso  |  Dir.Ini  |  Dir.Fin  |  Dir.Act  |");
         for(m=0;m<np;m++)
         {
            mproceso = colap[m]+1;
            mdirini = vpcb[mproceso-1].di;
            mdirfin = vpcb[mproceso-1].df;
            mdiract = vpcb[mproceso-1].da;
            printf("|      %Lu     |   %Lu    |   %Lu    |   %Lu    |",mproceso,mdirini,mdirfin,mdiract);
         }
         printf("*");
      activar_timer();
      comando='D';
      valor2=5;
      int16 mm;
      for (mm=0;mm<250;mm++)
      {
         int mmm;
         for (mmm=0;mmm<250;mmm++)
         {
            mm=mm+1;
            mm=mm-1;
         }
      }
   }
   
}
*/

void limpiar_calendario (int mcm){
   for (int i=0; i<mcm; i++){
      Calendario[i] = 0;
   }
}

void cargar_proceso(int idp, int prioridad)
{
    colap[np]=idp-1;
    vpcb[idp-1].estado=1;
    vpcb[idp-1].prioridad =prioridad;
    proceso=10;
    np++;
}

void descargar_proceso(int idp)
{
   int s,d,f;
   for(s=0;s<np;s++)
      {if(colap[s]==idp-1)
         {d=s;}
      }
   f=np-1;
   for(s=d;s<f;s++)
      {colap[s]=colap[s+1];
      }
   np--;   
   vpcb[idp-1].estado=0;
   proceso=0;
}

void Port_Init()
{
   set_tris_b(0x00);
   set_tris_d(0x00);
   set_tris_a(0x00);
   output_b(0x00);
   output_d(0x00);
   output_a(0x00);
}

void Config_TMR0()
{
   T0CON=0b10000111;

}

void Procesos_Init()
{
   //Programa 1
   vpcb[0].di=0x1000;
   vpcb[0].da=0x1000;   
   vpcb[0].df=0x1044;
   //Programa 2
   vpcb[1].di=0x1046;
   vpcb[1].da=0x1046;
   vpcb[1].df=0x1090;
   //Programa 3
   vpcb[2].di=0x1092;
   vpcb[2].da=0x1092;
   vpcb[2].df=0x10D6;
   //Programa 4
   vpcb[3].di=0x10D8;
   vpcb[3].da=0x10D8;
   vpcb[3].df=0x1128;
   //Programa 5
   vpcb[4].di=0x112A;
   vpcb[4].da=0x112A;
   vpcb[4].df=0x117A;
   //Programa 6
   vpcb[5].di=0x117C;
   vpcb[5].da=0x117C;
   vpcb[5].df=0x11CC;
   //Programa 7
   vpcb[6].di=0x11CE;
   vpcb[6].da=0x11CE;
   vpcb[6].df=0x121E;
   
   vpcb[0].estado=0;
   vpcb[1].estado=0;
   vpcb[2].estado=0;
   vpcb[3].estado=0;
   vpcb[4].estado=0;
   vpcb[5].estado=0;
   vpcb[6].estado=0;

   
}

void MCU_Init()
{
   printf("Conectado.............*");
   Port_Init();
   Procesos_Init();
   inte_Init();
   Config_TMR0();
   setup_spi(FALSE);
   setup_psp(PSP_DISABLED);
   setup_comparator(NC_NC_NC_NC);
   setup_vref(FALSE);
   port_b_pullups(FALSE);
}


#ORG 0x4000, 0x4FFE
void Inicio_SO()
{
   while (TRUE)
   {       
      if(flagcmd){
         procesa_cmd();
      }
      switch(comando)
      {
         case 'C': {    // Cargar proceso especifico
                     if (valor2<6) cargar_proceso(valor2 , 1);
                     break;
                   }
         case 'D': {    // Descargar proceso especifico
                     if (valor2<6) descargar_proceso(valor2);
                     break;
                   }
         case 'U': {    // Cargar procesos 1-6
                     for (m=1;m<6;m++) {cargar_proceso(m , 1);}
                     break;
                   }
         case 'X': {    // Descargar procesos 1-6
                     for (m=1;m<6;m++) {descargar_proceso(m);}
                     break;
                   }
         case 'M': {    // iniciar/detener carga del calendario
                     if (valor2 == 0) {
                        estado_calendario = 3;
                        mcm = (val1*255) + val2;
                        limpiar_calendario (mcm);
                        printf("M+0*");
                     } else if ( valor2 == 1 ){
                        estado_calendario = 0;
                        printf("M+1*");
                     }
                     break;
                   }
         case 'P': {    // Guardar un proceso en el calendario
                     int pos;
                     pos = (val1*255) + val2;
                     Calendario[pos] = valor2;
                     printf("P+%i*",valor2);
                     break;
                   }
         case 'T': {    // Ejecutar calendario
                     estado_calendario = 1;
                     reloj = 0;
                     printf("T+1*");
                     break;
                   }
         case 'R': {    // Detener Calendario
                     estado_calendario = 0;
                     printf("R+1*");
                     break;
                   }
      }
      comando='-';
      valor2=100;
      if (estado_calendario == 1){
         if (Calendario[reloj] != 0){
            cargar_proceso(Calendario[reloj] , 2);
         }
      }
      if(np>0)
      {
         if(vpcb[colap[0]].estado==1)
            {activar_timer();
             vpcb[colap[0]].estado=2;
             goto_address(vpcb[colap[0]].di);
            }
         else
            {wreg=vpcb[colap[0]].wreg;
             activar_timer();
             goto_address(vpcb[colap[0]].da);
            }
      }
   }
}

void main(void)
{
   printf("Esperando conexi�n..*");
   
   MCU_Init();
   #asm
   call 0x4000
   #endasm
}

