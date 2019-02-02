function  Distances  = Calc_Distancia(Rotula,Puntos_Moved_Red)

Fixed = Puntos_Moved_Red(:,:,2*Rotula-1); % 

% Para cada agujero, calcula la posici�n de la placa m�vil seg�n h y los
% �ngulos alpha1 y alpha2.

Moved = Puntos_Moved_Red(:,:,2*Rotula); % 

% C�lculo de las distancias entre cada par de agujeros mediante el c�lculo
% de la distancias entre dos puntos (m�dulo).
Distances = sqrt(sum((Moved'-Fixed').^2));
%end

