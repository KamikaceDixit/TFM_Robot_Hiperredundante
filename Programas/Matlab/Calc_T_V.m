function [Time,V_Tr] = Calc_T_V(Vmax,AcelT,DcelT,PulsesIni,PulsesFin,v_0,v_1,a_0,a_1)
%Poli4_Pos(t0,t1,x_0,v_0,v_1,a_0,a_1)
%Poli1_Pos(t0,x_0,v_0)
%Rep_Tr(Tr1,Tr2,Tr3,t0,t1,t2,t3)
%Pulses(t0,t1,x_0,v_0,v_1,a_0,a_1)

%Esta función calcula la velocidad y el tiempo de la trayectoria en cada
%caso.

%Si le mando que vaya para atrás, la velocidad debe ser negativa. Vmax es
%la velocidad general del robot, en valor absoluto.
if (PulsesFin<PulsesIni)
   Vmax=-Vmax; 
end

PulsesAcel=Pulses(0,AcelT,0,v_0,Vmax,a_0,a_1);
PulsesDcel=Pulses(0,DcelT,0,Vmax,v_1,a_0,a_1);
PulsesAcelDcel=PulsesAcel+PulsesDcel;

if(abs(PulsesAcelDcel)<=abs((PulsesFin-PulsesIni)) )
%%En este caso la trayectoria se divide en 3 tramos, uno de
%%aceleracion,otro de v=cte=vmax y el ultimo de deceleracion, ya que
%%significa que llega a alcanzar la velocidad máxima.

    Time=AcelT+DcelT+(PulsesFin-PulsesIni-PulsesAcelDcel)/Vmax;
    V_Tr=Vmax;
    
    
%     Tr1=Poli4_Pos(0,AcelT,PulsesIni,0,Vmax,0,0);
%     Tr2=Poli1_Pos(0,PulsesIni+PulsesAcel,Vmax);
%     Tr3=Poli4_Pos(0,DcelT,PulsesFin-PulsesDcel,Vmax,0,0,0);
%     syms t;
%     figure
%     hold on
%     Rep_Tr(Tr1,Tr2,Tr3,0,AcelT,Time-DcelT,Time);
%     Rep_Tr(diff(Tr1,t),diff(Tr2,t),diff(Tr3,t),0,AcelT,Time-DcelT,Time);
%     legend("Posición","Velocidad");
%     title("Posición y velocidad");
%     xlabel("Tiempo");
%     ylabel("Pulsos - Pulsos/s");
  
elseif (abs(PulsesAcelDcel)>abs(PulsesFin-PulsesIni))
%%En este caso, no da tiempo a llegar a la velocidad maxima, por lo que
%%no existirá tramo a v=cte. Habra que hayar la velocidad critica que
%%hace llegar al punto de destino en el menor tiempo posible.

    %PulsosTotales=Pulses(0,AcelT,0,0,v,0,0)+Pulses(0,DcelT,0,v,0,0,0);
    %V_Tr=double(solve(PulsosTotales-PulsesFin+PulsesIni,v));
    V_Tr=(2*PulsesFin - 2*PulsesIni)/(AcelT + DcelT); %%Ojo que solo vale para v0=0 y vf=0,a0 y af=0
    %En el caso de no querer parar al llegar al final y así tener un
    %movimiento más fluido entre trayectorias, la Vf no sería 0, habría que
    %utilizar simbólicas, hay que despejarlas ya que es muy lento.
    
       if (V_Tr==0) %Caso particular si le mando que vaya al punto en el que ya está.
           Time=0;
       else
           
    PulsesAcel=Pulses(0,AcelT,0,0,V_Tr,0,0);
    PulsesDcel=Pulses(0,DcelT,0,V_Tr,0,0,0);
    PulsesAcelDcel=PulsesAcel+PulsesDcel;
    Time=AcelT+DcelT+(PulsesFin-PulsesIni-PulsesAcelDcel)/V_Tr;
       end
       
       
% figure
% Tr1=Poli4_Pos(0,AcelT,PulsesIni,0,V_Tr,0,0);
% Tr2=Poli1_Pos(0,PulsesIni+PulsesAcel,V_Tr);
% Tr3=Poli4_Pos(0,DcelT,PulsesFin-PulsesDcel,V_Tr,0,0,0);
% Rep_Tr(Tr1,Tr2,Tr3,0,AcelT,Time-DcelT,Time);
% hold on
% Rep_Tr(diff(Tr1,t),diff(Tr2,t),diff(Tr3,t),0,AcelT,Time-DcelT,Time);


end
end
