function [Output] = Serial(Arduino,Input,varargin)
%%Comun

Mot_I = [ 1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23,25,26,28,29,31,32,34,35 ];

p=inputParser;
addRequired(p,'Arduino')
addRequired(p,'Input');
if (strcmp(Input,'Conf'))
    addRequired(p,'Time',@(x) (x>=0) && isnumeric(x) && isreal(x));
end
if (strcmp(Input,'Tr'))
    addRequired(p,'V_Tr');
end
addParameter(p,'Speed','0');
addParameter(p,'AcelT','0.5');
addParameter(p,'DcelT','0.5');
parse(p,Arduino,Input,varargin{:});   


switch p.Results.Input
    case "Conf"
        %% Configuración
        %Se envia la configuracion general de la trayectoria, estos
        %parametros son comunes a todas las trayectorias.
        flushinput(Arduino); %Borra todo lo que haya en el buffer Serial.
        fprintf(Arduino,'%s\n','#Conf_1');
        fprintf(Arduino,'%f\n',p.Results.Speed);
        fprintf(Arduino,'%f\n',p.Results.AcelT);
        fprintf(Arduino,'%f\n',p.Results.DcelT);
        fprintf(Arduino,'%f\n',p.Results.Time);
        fprintf(Arduino,'%s\n','#Conf_#');
    
    case "Pos"
        %clc
        %% Posiciones
        % Se solicita la posicion exacta en pulsos de cada uno de los
        % motores y se guarda en Output.
        while Arduino.BytesAvailable
            flushinput(Arduino);
        end

        Output= zeros(1,36);
        fprintf(Arduino,'%s\n','#Pos');%%Peticion de posiciones
        pause(0.05);
        for tryn=1:20
            if Arduino.BytesAvailable
                for i=1:size(Mot_I,2)
                    Output(Mot_I(i)) = fscanf(Arduino,'%f');
                end
                break;
            else
                pause(0.2);
            end
        end
        
        if fscanf(Arduino,'%s')=="Errors"
          Input_Text=fscanf(Arduino,'%s');
          disp(['Errores de recepción: ' Input_Text]);
          if(Input_Text~="0")
              disp(['Problemas en Drivers: ' fgetl(Arduino)]);
          end
        else
          disp("Mensaje Corrupto")
        end
        
    case "Tr"
        %% Trayectorias
        flushinput(Arduino);
        fprintf(Arduino,'%s\n','#Tr_1');
        for i=1:size(Mot_I,2)
            fprintf(Arduino,'%.3f\n',p.Results.V_Tr(Mot_I(i)));
        end
        fprintf(Arduino,'%s\n','#Tr_#');
        
    case "Ini"
        %% Comienzo
        flushinput(Arduino);
        fprintf(Arduino,'%s\n','#Ini');     
        
    otherwise
        disp("Parametro Incorrecto");
end

end