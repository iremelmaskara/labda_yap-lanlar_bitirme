%% KOD DÜZENLEME 
%% Parametre değerleri
Ea=24;
E_max=24;
E_min=-24;
Raa=2.8;  
Laa=1.4;  
ja=0.72;
BB=0.02;
Kmm=1.24; 
Kbb=1.24;
Kb_rpm =Kbb*2*pi/60;
kt=10;
s=tf('s');
%% sürekli hal hız hatası isterleri
Kh=0.1;  %%bu ess nin katsayısı gibi ama neden 
%% ksi ve wn hesabı 
a=pi^2+log(yuzde_asim/100)^2;
ksi=-log(yuzde_asim/100)/sqrt(a);
wn=4/(ksi*ts);
%% ACTF S-domeni
n=0.95;
d=[102e-3 1];
ACTF=tf(n,d);
%% Örnekleme zamanının seçimi 
kok=roots(d);
min_kok=max(abs(kok));
tomin=1/max(abs(kok));
Ts=10e-3;
%% geçici rejim kriterleri (isterler)
yuzde_asim=4.32; 
ts= 4/min_kok; 
%% ACTF Z-dönüşüm
z=tf('z',Ts);
ACTF_Z= c2d(ACTF,Ts);
[n_z,d_z]=tfdata(ACTF_Z,'v');
%% baskın kök hesabı
karak_denk=[1 2*ksi*wn wn^2];
baskin_kok=roots(karak_denk);
z_baskin_kok=exp(baskin_kok*Ts);
z1= z_baskin_kok(1);
%% Ki yi bulmak için
ess_hesap_s=Ea*2*ksi/wn;
Kv=Ea/ess_hesap_s;
%%
nss=0.95;
dss=[102e-3 1];
Gss=tf(nss,dss);
Gzz=c2d(Gss,Ts);
[nzz,dzz]=tfdata(Gzz,'v');
%% 
Ki=Kv*Ts/(polyval(nzz,1)/polyval(dzz,1));
%% Kp,Kd hesaplama
genlik_z1=abs(z1); % Genlik (|z1|)
beta=angle(z1);    % Açı (radyan)

Gp_z1= evalfr(ACTF_Z, z1); % z = z1 için transfer fonksiyonu değeri
genlik_Gp_z1=abs(Gp_z1);    % Genlik (|z1|)
gama=angle(Gp_z1);    % Açı (radyan)

A=-cos(gama)/genlik_Gp_z1;
B=-2*Ki*genlik_z1*(genlik_z1-cos(beta)/(genlik_z1^2-2*genlik_z1*cos(beta)+1));
C=(-genlik_z1*sin(gama)+cos(beta)*sin(gama))/(genlik_Gp_z1*sin(beta));
Kp=A+B+C;

D=genlik_z1/sin(beta);
H=Ki*sin(beta)/(genlik_z1-2*cos(beta)+(1/genlik_z1));
F=sin(gama)/genlik_Gp_z1;
Kd=D*(H+F);

% sim('iremelmas_tasarim.slx')