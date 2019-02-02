clear;
%dejdkajsda
%syms t0 t1 a b c d v0 v1 a0 a1 t v e;

%% Variables de entrada
t0=0.3;t1=5.7;v0=0;v1=5000;a0=0;a1=0;x0=0;x1=0;


%% Calculo de parametros
a=(2*t0*v1 - 2*t0*v0 + 2*t1*v0 - 2*t1*v1 + a0*t0^2 + a0*t1^2 + a1*t0^2 ...
    + a1*t1^2 - 2*a0*t0*t1 - 2*a1*t0*t1)/(t0^4 - 4*t0^3*t1 + 6*t0^2*t1^2 ...
    - 4*t0*t1^3 + t1^4);

b=-(a0*t0^3 + 2*a0*t1^3 + 2*a1*t0^3 + a1*t1^3 - 3*t0^2*v0 + 3*t0^2*v1 + ...
    3*t1^2*v0 - 3*t1^2*v1 - 3*a0*t0*t1^2 - 3*a1*t0^2*t1)/(t0^4 - ...
    4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

c=(a0*t1^4 + a1*t0^4 + 2*a0*t0^3*t1 + 2*a1*t0*t1^3 + 6*t0*t1^2*v0 - ...
    6*t0^2*t1*v0 - 6*t0*t1^2*v1 + 6*t0^2*t1*v1 - 3*a0*t0^2*t1^2 - ...
    3*a1*t0^2*t1^2)/(t0^4 - 4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

d=(t0^4*v1 + t1^4*v0 - a0*t0*t1^4 - a1*t0^4*t1 - 4*t0*t1^3*v0 - ...
    4*t0^3*t1*v1 + 2*a0*t0^2*t1^3 - a0*t0^3*t1^2 - a1*t0^2*t1^3 + ...
    2*a1*t0^3*t1^2 + 3*t0^2*t1^2*v0 + 3*t0^2*t1^2*v1)/(t0^4 - ...
    4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

e=x0-(1/4)*a*t0^4-(1/3)*b*t0^3-(1/2)*c*t0^2-d*t0;

%% Generacion de Curvas
t=t0:(t1-t0)/100:t1;

Pos=(1/4)*a*t.^4+(1/3)*b*t.^3+(1/2)*c*t.^2+d*t + e;
Vel=a*t.^3+b*t.^2+c*t+d;
Acel=3*a*t.^2+2*b*t+c;

%% Representacion
figure
hold on

plot(t,Vel);
%plot(t,Pos);
plot(t,Acel*0.1);