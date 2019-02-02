clear
clc

% global alpha beta
global Mot_I;
%%Configuration Parameters
Vmax=1000;
AcelT=0.4;
DcelT=0.4;

%%Variables de entrada
alpha=[0 0 45 0 0 0 0 0];    %Direccion de maxima pendiente
beta=[0 0 0 0 0 0 0 0];     %Angulo de giro


delete(instrfindall);
Arduino = serial("COM16",'BaudRate',115200);
set(Arduino,'DataBits',8);
set(Arduino,'StopBits',1);
set(Arduino,'Parity','none');
set(Arduino,'Timeout',0.5);
fopen(Arduino);
pause(2);

%%------------------------
PulsesIni=Serial(Arduino,"Pos")

Mot_I = [ 1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23,25,26,28,29,31,32,34,35 ];
V_Tr=zeros(1,36);

Puntos_Moved_Red=Calc_Posiciones(alpha,beta);
Pulsos_Destino = Calc_Pulsos(Puntos_Moved_Red);
%Dibujar_Robot(Puntos_Moved_Red);

[M,I] = max(abs(Pulsos_Destino-PulsesIni));

[Time,] = Calc_T_V(Vmax,AcelT,DcelT,PulsesIni,Pulsos_Destino(I),0,0,0,0);


for i=1:24
%V_Trs(Mot_I(i))=double(Calc_V(AcelT,DcelT,PulsesIni,Pulsos_Destino(Mot_I(i)),Time));
V_Tr(Mot_I(i))=(2*PulsesIni - 2*Pulsos_Destino(Mot_I(i)))/(AcelT + DcelT - 2*Time);
end

Serial(Arduino,"Conf",Time,'Speed',Vmax,'AcelT',AcelT,'DcelT',DcelT);
Serial(Arduino,"Tr",V_Tr);

        for i=1:20
        if Arduino.BytesAvailable
        Output = fscanf(Arduino,'%s')
        else
            pause(0.02);
        end
        end


fclose(Arduino);