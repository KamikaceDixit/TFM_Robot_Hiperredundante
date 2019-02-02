function [Puntos_Moved_Red] = Calc_Posiciones(alpha,beta)
R=50;   %mm Radio al punto medio de los agujeros guía de los cables.
H=50;   %mm Distancia entre placas de sección articulada.
H2=58;   %mm Distancia entre placas rigidas.
Num_Agujeros=36; %Agujeros distribuidos uniformemente en el disco.
Num_Rotulas=8;
Posicion_eje_y=27.5;    % Ubicación del eje y con respecto a la numeración 
                        % de los agujeros, en este casos se encuentra entre 
                        % el agujero 27 y el 28.
                        

Punto=[0 R 0 1]; %x y z
Punto= rotz((Posicion_eje_y-1)*10)*Punto';%% hay que tocar este para que coincida con el 1
Puntos_Fixed=zeros(Num_Agujeros,4);
Puntos_Moved=zeros(Num_Agujeros,4,Num_Rotulas*2);
Puntos_Moved(:,4,:)=1;

for i=1:Num_Agujeros
    Puntos_Fixed(i,1:4)=(rotz(-10*(i-1))*Punto);
end



%%Nuevo
Ejes=zeros(Num_Rotulas,3);

%%Se crean los vectores de giro para Rodrigues
for i=1:Num_Rotulas
    Ejes(i,:)=[cos(deg2rad(90+alpha(i))),sin(deg2rad(90+alpha(i))),0]; 
end

%%Se guardan las posiciones de todos los puntos
for j=Num_Rotulas:-1:1
     Puntos_Moved(:,:,2*j-1)=Puntos_Fixed(:,:);
     R=Rodrigues(Ejes(j,:),beta(j));
    for i=1:Num_Agujeros
        Puntos_UP=trasz(H/2)*Puntos_Fixed(i,:)';
        Puntos_Moved(i,1:3,2*j)=(R*Puntos_UP(1:3));
        Puntos_Moved(i,:,2*j)=trasz(H/2)*Puntos_Moved(i,:,2*j)';
    end
    for k=(2*j)+1:2*Num_Rotulas  
        for p=1:Num_Agujeros
        Aux=trasz(H2+H/2)*Puntos_Moved(p,:,k)';
        Puntos_Moved(p,:,k)=Aux(:)';
        Puntos_Moved(p,1:3,k)=R*Puntos_Moved(p,1:3,k)';
        Puntos_Moved(p,:,k)=trasz(H/2)*Puntos_Moved(p,:,k)';
        end
    end
end

Puntos_Moved_Red=Puntos_Moved(:,1:3,:);
%Puntos_Moved_Red son todos los puntos eliminando la 4 componente de
%escala.
end


