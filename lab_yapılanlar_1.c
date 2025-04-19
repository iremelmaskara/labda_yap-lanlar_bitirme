#include <ADUC841.h>

//////////////// Tanimlamalar ///////////////////////////////////////
sbit LED = P0^1;  // LED P0.1 pinine tanimlandi
sbit led_ayar=P2^3;
sbit buton_2= P2^1;

void timer_ayar(void);
void kesme_ayar(void);
void ADC_ayar(void);
void DAC_ayar(void);
void hata_hesapla(void);
void timer_baslat(void);
//void send_data(unsigned char c);
void DAC0_yaz(int deger);
//void seri_port_ayar(void);
int PID_hesapla(int ref, int cikis);

unsigned char ref_h;
unsigned char ref_l;
unsigned char cikis_h;
unsigned char cikis_l;
unsigned char hata_h;
unsigned char hata_l;
unsigned char kontrol_int_h;
unsigned char kontrol_int_l;


unsigned int ref;
unsigned int cikis;
unsigned int ref_oku(void);
unsigned int cikis_oku(void);

int hata, kontrol_int;
float kontrol_P , kontrol_I , kontrol_D , kontrol ;
int hata_eski=0;
int hata_toplam=0;

//***********PID katsayilari****
float Kp = 0.5;
float Ki = 0.1032;
float Kd = -4.3977;
//***************** kesme ayarlari ***************************

void Timer0_kesmesi(void) interrupt 1 
{ 
    static unsigned int dongu_sayisi = 0; 
    dongu_sayisi++;

    if (dongu_sayisi >=10) 
			{ 				
				////////GOREV //////////////////////
				LED = ~LED;  
				
				ref=ref_oku();    // pot1 deki degeri oku 
				cikis=cikis_oku();  // pot2 deki degeri oku
				
				ref_h=(ref >> 8) & 0xFF;
				ref_l= ref & 0xFF;
				cikis_h=(cikis >> 8)& 0xFF;
				cikis_l= cikis & 0xFF;
				
				hata=ref-cikis;
				hata_h=(hata >> 8)& 0xFF;
				hata_l= hata & 0xFF;
				
				kontrol_int = PID_hesapla(ref,cikis);
				
				kontrol_int_h=(kontrol_int >> 8) & 0xFF;
				kontrol_int_l= kontrol_int & 0xFF;
				
				DAC0_yaz(kontrol_int);
				
//				send_data('s');
//				send_data(ref_l);	
//				send_data(ref_h);
//				send_data(cikis_l);	
//				send_data(cikis_h);
//				send_data(hata_l);	
//				send_data(hata_h);
//				send_data(kontrol_int_l);	
//				send_data(kontrol_int_h);
//				send_data('e');
		
				/////////////////////////////////////
			  dongu_sayisi = 0;
			 }

    TH0 = 0xD4;        // Timer tekrar baslatilmasi için degerler
    TL0 = 0xCC;
    TF0 = 0;           // Timer0 bayragini temizle
}

//************** ADC okuma fonksiyonlari **********************
unsigned int ref_oku(void) 
{
		unsigned int ref;
	
	  ADCCON2=0x07; //adc 0 kanali seçtik.
	  SCONV=1;
	  while(SCONV==1){};
		
		
		ref=((ADCDATAH & 0x0f)<<8) + ADCDATAL;  
	  return ref;  
}	

unsigned int cikis_oku(void) 
{
		unsigned int out;
	  ADCCON2=0x02; //adc 1 kanali seçtik.
	  SCONV=1;
	  while(SCONV==1){};
		
		out=((ADCDATAH & 0x0f)<<8) + ADCDATAL;   
	  return out;  
}

//********************* DAC yazma fonksiyonu ************************ 
void DAC0_yaz(int deger)
{     //Dac0=Kontrol_p i yazdiriyoruz
	if(deger<0)
	{
		deger=0;
	}
	if(deger>4095)
	{
		deger=4095;
	}
	//deger=2048;
	DAC0H=((deger >> 8) & 0x000F);
	DAC0L=deger;
}
//***********************************************
void main(void)
{
		while(buton_2==1){};
	//butan bas birak algilanacak öyle devam edilecek
//		led_ayar=0;
//    P0 = 0x00; 
	
		timer_ayar();
		kesme_ayar();
		ADC_ayar();  
		DAC_ayar();
		timer_baslat();
//		seri_port_ayar();
	
		TR1=1;  // tmr baslat

		while (1){};
}
void timer_ayar(void)
{
	 TMOD = 0x01;      // Timer0 mod 1 (16-bit timer)
   TH0 = 0xD4;      // 65535 - 11059 = 54476 -> D4CC (hex)
   TL0 = 0xCC;
}
void kesme_ayar(void)
{
	 ET0 =1;         // Timer0 kesmesini aktif et
   EA = 1;          // Genel kesmeleri aktif et
}
void ADC_ayar(void)
{
	 ADCCON1=0xFC; //1(enerji)0(dahili)00 0011   dahili adc kullandim
}
void DAC_ayar(void)
{
	 DACCON=0x7F; // dac0 (0000-1001 b)(senkron kapali)
}

void timer_baslat(void)
{
	 TR0 = 1;         // Timer0 baslat
}

////////// seri haberlesme fonksiyonlari ////////////////////
//void send_data(unsigned char c)
//{
//		SBUF=c;
//		while(TI==0); // veri giti mi bayragi kontrol ediliyor.
//		TI=0;
//}	
//void seri_port_ayar(void)
//{
//	TMOD=0x20; // oto yuklemeli tmr1 mode2 0010 0000
//	SCON=0x50; // mode_1 8 bit uart  (RI(geldi mi) TI(gitti mi) 0 0 1 mode_1(8bit))
//	TH1=0xDC; // 9600 baud rate için hesaplanan deger

//}
//////////////// PID hesap fonksiyonu //////////////
int PID_hesapla(int ref, int cikis)
{
	hata=ref-cikis;
	
	hata_toplam += hata;  //hata_toplam=xi demek
	kontrol_P = Kp*hata;
	kontrol_I = Ki * hata_toplam;
	if (hata_toplam>4095/Ki)
	{
		hata_toplam = 4095/Ki; 
	}

	if (hata_toplam<-4095/Ki)
	{
		hata_toplam = -4095/Ki; 
	}
	kontrol_D = Kd * (hata - hata_eski);

	kontrol = kontrol_P + kontrol_I + kontrol_D;  //PID cikisi 

	
	// Eski hatayi güncelle
	hata_eski = hata;

	
	// Sinirlandirma (0-4095 arasi deger)
	if (kontrol < 0)
		kontrol = 0;
	if (kontrol > 4095)
		kontrol = 4095;
kontrol_int=(int)kontrol;
	return kontrol_int;
}